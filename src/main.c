/*
Cloned from quickstart `https://github.com/raylib-extras/raylib-quickstart` by Jeffery Myers, marked with CC0 1.0.
To view a copy of this license, visit https://creativecommons.org/publicdomain/zero/1.0/
*/

#include <stdlib.h>
#include "raylib.h"
#include "raygui.h"
#include "rlgl.h"
#include "constants.h"
#include "resource_util.h"
#include "data/app_configs/app_config.h"
#include "data/chart_settings/chart_settings.h"
#include "gameplay/audio_service.h"
#include "gameplay/camera/camera_service.h"
#include "gameplay/chart_reader.h"
#include "render/render_service.h"
#include "render/texture/texture_service.h"
#include "render/hud/hud_services.h"
#include "render/notes/notes_service.h"
#include "render/track/track_service.h"
#include "render/utils/drawing.h"
#include "windows/window_inst.h"
#include "windows/window_services.h"

#define GLSL_VERSION 330

int main()
{
  rini_data  rini_d      = fetch_rini_config();
  AppConfigs app_configs = app_configs_init(&rini_d);

  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(BASE_APP_WINDOW_WIDTH,
             aspect_ratio_get_height((float)BASE_APP_WINDOW_WIDTH, app_configs.playfield_ratio),
             "arkohra");
  SetExitKey(KEY_NULL);

  // Now a special case a kohra
  TextureGroup texture_group = textures_init();
  ChartReader  chart_reader  = { 0 };
  TrackService track_service = track_service_init();
  NotesService notes_service = notes_service_init(GLSL_VERSION);
  HudService   hud_service   = hud_service_init(GLSL_VERSION);
  WindowGroup  window_group  = window_services_init(GLSL_VERSION, &app_configs);

  RenderContext render_ctx = {
    .camera         = camera_init_playfield(),
    .chart_settings = chart_settings_init(&app_configs),
    .audio_clock    = { 0 },
  };

  InitAudioDevice();
  Wave wave = LoadWave(render_ctx.chart_settings.audio_path);
  float wave_time_ms = ((float)wave.frameCount * 1000) / (float)(wave.sampleRate);

  Music music = LoadMusicStream("");
  music.looping = false;
  SetMusicPan(music, 0.0f);
  SetMusicVolume(music, app_configs.music_volume);
  PlayMusicStream(music);
  PauseMusicStream(music);
  audio_clock_pause(&render_ctx.audio_clock);

  SetTargetFPS(60);
  rlSetClipPlanes(0.01f, 100.0f);

  while (!WindowShouldClose())
  {
    if (IsFileDropped())
    {
      // Pause audio before processing
      PauseMusicStream(music);
      audio_clock_pause(&render_ctx.audio_clock);

      // Parse and repolulate chart settings
      FilePathList dropped_file = LoadDroppedFiles();
      char *project_file = dropped_file.paths[0];
      if (TextIsEqual(GetFileName(project_file), "project.json"))
      {
        if (chart_settings_load_from_project(&render_ctx.chart_settings, &app_configs, project_file))
        {
          // Stop audio entirely
          render_ctx.audio_clock.total_audio_length = 0;
          StopMusicStream(music);
          UnloadMusicStream(music);
          music = LoadMusicStream(render_ctx.chart_settings.audio_path);
          music.looping = false;
          render_ctx.audio_clock.total_audio_length = (int)roundf(GetMusicTimeLength(music) * 1000);
          SetMusicPan(music, 0.0f);
          SetMusicVolume(music, app_configs.music_volume);

          // Update chart reader
          if(chart_reader.initialized) chart_reader_unload(&chart_reader);

          chart_reader = chart_reader_parse(&render_ctx.chart_settings,
                                            &render_ctx.audio_clock,
                                            &notes_service.arc_tex,
                                            &notes_service.arc_shader.shader);
          chart_settings_print(&render_ctx.chart_settings);

          track_service_apply_chart(&track_service, &render_ctx.chart_settings);
          notes_service_apply_chart(&notes_service, &render_ctx.chart_settings);
          hud_service_apply_chart  (&hud_service  , &render_ctx.chart_settings);

          PlayMusicStream(music);
          PauseMusicStream(music);
          audio_clock_start(&render_ctx.audio_clock);
          audio_clock_pause(&render_ctx.audio_clock);
        }
      }
      UnloadDroppedFiles(dropped_file);
    }

    camera_handle_main_window_resize(&render_ctx.camera, app_configs.playfield_ratio);
    window_services_handle_inputs(&window_group);

    // TODO: currently scrolling with constant speed, find a way to speed up/slow down based on first timing group's current bpm
    if (render_ctx.audio_clock.is_playing)
    {
      // Playfield: Track & Single Line Scrolling
      static float scroll_offset = 0.0f;
      scroll_offset += GetFrameTime() * render_ctx.chart_settings.scroll_speed;
      renderable_update_scroll(&track_service.track      , scroll_offset);
      renderable_update_scroll(&track_service.single_line, scroll_offset);
    }

    if (IsMusicValid(music))
    {
      UpdateMusicStream(music);

      if (!IsMusicStreamPlaying(music) && render_ctx.audio_clock.is_playing)
        audio_clock_pause(&render_ctx.audio_clock);

      if (IsKeyPressed(KEY_Q))
      {
        if (render_ctx.audio_clock.is_playing) { PauseMusicStream(music) ; audio_clock_pause(&render_ctx.audio_clock) ; }
        else                                   { ResumeMusicStream(music); audio_clock_resume(&render_ctx.audio_clock); }
      }
      if (IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_Q))
      {
        StopMusicStream(music);
        PlayMusicStream(music);
        PauseMusicStream(music);
        audio_clock_start(&render_ctx.audio_clock);
        audio_clock_pause(&render_ctx.audio_clock);
      }
    }

    // Reload config.ini
    if ((IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER)) &&
        (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) &&
        IsKeyPressed(KEY_R))
    {
      // Pause audio before processing
      PauseMusicStream(music);
      audio_clock_pause(&render_ctx.audio_clock);
      rini_d = fetch_rini_config();
      app_configs = app_configs_init(&rini_d);
      SetWindowSize(GetScreenWidth(), (int)aspect_ratio_get_height((float)GetScreenWidth(), app_configs.playfield_ratio));
      recalibrate_camera(&render_ctx.camera);
      render_ctx.chart_settings.scroll_speed = app_configs.scroll_speed;
      SetMusicVolume(music, app_configs.music_volume);

      bool is_setting_open = window_group.project_setting.is_visible;
      window_group = window_services_init(GLSL_VERSION, &app_configs);
      if (is_setting_open && !window_group.project_setting.is_visible) window_inst_toggle(&window_group.project_setting);

      // Update chart reader to regenerate arc meshes
      if(chart_reader.initialized) chart_reader_unload(&chart_reader);
      chart_reader = chart_reader_parse(&render_ctx.chart_settings,
                                        &render_ctx.audio_clock,
                                        &notes_service.arc_tex,
                                        &notes_service.arc_shader.shader);
    }

    // KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA
    if ((IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER)) &&
        (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) &&
        IsKeyPressed(KEY_K))
      app_configs.kohra = !app_configs.kohra;
    // KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA

    BeginDrawing();
      ClearBackground(WHITE);

      // Order:
      // Base track -> Notes -> Sky Input/Label -> HUD -> Windows
      track_service_render_base_track(&track_service, &render_ctx, &chart_reader.render_lists.enwidencamera_channel);
      if (chart_reader.initialized) notes_service_render(&notes_service, &render_ctx, &chart_reader);
      track_service_render_sky_input(&track_service, &render_ctx, &chart_reader.render_lists.enwidencamera_channel);
      hud_services_render(&hud_service, &render_ctx.chart_settings);
      windows_services_render(&window_group, &app_configs, &render_ctx);

      // Debug FPS
      DrawFPS(5, 5);

      // KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA
      if (app_configs.kohra)
      {
        BeginMode3D(render_ctx.camera);
          DrawCubeTexture(texture_group.kohra, (Vector3){0.0f, KOHRA_SIZE, 0.0f}, KOHRA_SIZE, KOHRA_SIZE, KOHRA_SIZE, WHITE);
        EndMode3D();
      }
      // KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA

    // end the frame and get ready for the next one  (display frame, poll input, etc...)
    EndDrawing();
  }

  // Unload stuffs
  UnloadMusicStream(music);
  UnloadWave(wave);
  CloseAudioDevice();

  chart_reader_unload(&chart_reader);
  textures_unload(&texture_group);
  windows_services_unload(&window_group);

  // One last config writing just in case
  app_configs_write_to_file(&app_configs, &rini_d);

  // destroy the window and cleanup the OpenGL context
  CloseWindow();
  return 0;
}
