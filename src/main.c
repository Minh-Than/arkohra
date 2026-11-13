/*
Cloned from quickstart `https://github.com/raylib-extras/raylib-quickstart` by Jeffery Myers, marked with CC0 1.0.
To view a copy of this license, visit https://creativecommons.org/publicdomain/zero/1.0/
*/

#include "data/fonts/fonts_service.h"
#include "gameplay/arc_formula.h"
#include "raylib.h"
#include "raygui.h"
#include "raymath.h"
#include "rlgl.h"
#include "constants.h"
#include "data/app_configs/app_config.h"
#include "data/chart_settings/chart_settings.h"
#include "data/selection/selection_service.h"
#include "gameplay/audio_service.h"
#include "gameplay/camera/camera_service.h"
#include "gameplay/chart_reader.h"
#include "render/hud/hud_services.h"
#include "render/notes/notes_service.h"
#include "render/render_service.h"
#include "render/texture/texture_service.h"
#include "render/track/track_service.h"
#include "render/utils/drawing.h"
#include "windows/command_palette/window.h"
#include "windows/project_setting/window.h"
#include "windows/rgui_input_data.h"
#include "windows/window_inst.h"
#include "windows/window_services.h"

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION 330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION 100
#endif

// TODO: Is there a better way/place for this?
static void process_imported_project(const char* project_file, Music *music,
                                     AppConfigs *app_configs, CachedFontProbes *font_probes,
                                     TrackService *track_service, NotesService *notes_service, HudService *hud_service, WindowService *window_service,
                                     ChartReader *chart_reader, RenderContext *render_ctx, SelectionService *selection_service,
                                     Shader *chart_shader, int *timing_loc, int *screenSize_loc)
{
  if (TextIsEqual(GetFileName(project_file), "project.json"))
  {
    // Pause audio before processing
    PauseMusicStream(*music);
    audio_clock_pause(&render_ctx->audio_clock);

    if (chart_settings_load_from_project(&render_ctx->chart_settings, app_configs, project_file))
    {
      // Stop audio entirely
      render_ctx->audio_clock.total_audio_length = 0;
      StopMusicStream(*music);
      UnloadMusicStream(*music);
      *music = LoadMusicStream(render_ctx->chart_settings.audio_path);
      music->looping = false;
      render_ctx->audio_clock.total_audio_length = (int)roundf(GetMusicTimeLength(*music) * 1000);
      SetMusicPan(*music, 0.0f);
      SetMusicVolume(*music, app_configs->music_volume);

      selection_service_unload(selection_service);

      // Update chart reader
      if(chart_reader->initialized) chart_reader_unload(chart_reader);
      *chart_reader = chart_reader_parse(&render_ctx->chart_settings, &render_ctx->audio_clock);
      // chart_settings_print(&render_ctx->chart_settings);

      const char *fs_path = TextIsEqual(render_ctx->chart_settings.shader_fs_path, "")
                            ? TextFormat(DEFAULT_CHART_SHADER_FS, GLSL_VERSION)
                            : render_ctx->chart_settings.shader_fs_path;
      if (!(IsPathFile(fs_path) && IsFileExtension(fs_path, ".fs")))
        fs_path = TextFormat(DEFAULT_CHART_SHADER_FS, GLSL_VERSION);

      UnloadShader(*chart_shader);
      *chart_shader = LoadShader(TextFormat(DEFAULT_CHART_SHADER_VS, GLSL_VERSION), fs_path);
      *timing_loc     = GetShaderLocation(*chart_shader, "timing");
      *screenSize_loc = GetShaderLocation(*chart_shader, "screenSize");

      track_service_apply_chart(track_service, &render_ctx->chart_settings);
      notes_service_apply_chart(notes_service, &render_ctx->chart_settings);
      hud_service_apply_chart  (hud_service  , &render_ctx->chart_settings, font_probes);

      ProjSettingData *project_setting = (ProjSettingData *)window_service->project_setting.data;
      proj_setting_apply_chart(project_setting, &render_ctx->chart_settings, font_probes);

      PlayMusicStream(*music);
      PauseMusicStream(*music);
      audio_clock_start(&render_ctx->audio_clock);
      audio_clock_pause(&render_ctx->audio_clock);
    }
  }
}

int main()
{
  AppConfigs app_configs = app_configs_init();

  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
  InitWindow(app_configs.screen_width,
             aspect_ratio_get_height((float)app_configs.screen_width, app_configs.playfield_ratio),
             "arkohra");
  SetExitKey(KEY_NULL);

  CachedFontProbes font_probes       = cached_font_probes_init();
  TrackService track_service         = track_service_init  (GLSL_VERSION);
  NotesService notes_service         = notes_service_init  (GLSL_VERSION, &app_configs);
  HudService hud_service             = hud_service_init    (GLSL_VERSION, &app_configs, &font_probes);
  WindowService window_service       = window_services_init(GLSL_VERSION, &app_configs, &font_probes);
  SelectionService selection_service = selection_service_init();
  TextureGroup texture_group = textures_init(); // For now contains kohra

  // TODO: ChartReader + RenderContext as a whole feels too random
  ChartReader chart_reader  = { 0 };
  RenderContext render_ctx = render_context_init(&app_configs);


  // TODO: Figure out where to put these elsewhere more sensible:
  // - chart shader + uniform locations
  // - gameplay render texture
  // - music thing
  // - project file dialog input (drawing + handling)
  Shader chart_shader = LoadShader(TextFormat(DEFAULT_CHART_SHADER_VS, GLSL_VERSION),
                                   TextFormat(DEFAULT_CHART_SHADER_FS, GLSL_VERSION));
  int timing_loc     = GetShaderLocation(chart_shader, "timing");
  int screenSize_loc = GetShaderLocation(chart_shader, "screenSize");

  RenderTexture2D target = LoadRenderTexture(GetRenderWidth(), GetRenderHeight());
  GenTextureMipmaps(&target.texture);
  SetTextureFilter(target.texture, TEXTURE_FILTER_TRILINEAR);

  RguiFileInput open_proj_input = rgui_fileinput_init("");
  list_push(&open_proj_input.extensions, ".json");

  InitAudioDevice();

  Music music = { 0 };
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
    if ((IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER)) &&
        IsKeyPressed(KEY_O))
    {
      open_proj_input.state.windowActive = true;
    }
    if (IsFileDropped())
    {
      FilePathList dropped_file = LoadDroppedFiles();
      char *project_file = dropped_file.paths[0];
      process_imported_project(project_file, &music,
                               &app_configs, &font_probes,
                               &track_service, &notes_service, &hud_service, &window_service,
                               &chart_reader, &render_ctx, &selection_service,
                               &chart_shader, &timing_loc, &screenSize_loc);
      UnloadDroppedFiles(dropped_file);
    }

    camera_handle_main_window_resize(&render_ctx.camera, app_configs.playfield_ratio);
    window_services_handle_inputs(&window_service);
    selection_service_handle_inputs(&selection_service, &track_service, &window_service, chart_reader.initialized);
    window_services_window_check_oob(&window_service);

    track_service_update_scroll_offset(&track_service, &render_ctx, &chart_reader);

    if (target.texture.width != GetRenderWidth() || target.texture.height != GetRenderHeight())
    {
      UnloadRenderTexture(target);
      target = LoadRenderTexture(GetRenderWidth(), GetRenderHeight());
      SetTextureWrap(target.texture, TEXTURE_WRAP_CLAMP);
      SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);
    }

    if (IsMusicValid(music))
    {
      UpdateMusicStream(music);

      if (!IsMusicStreamPlaying(music) && render_ctx.audio_clock.is_playing)
        audio_clock_pause(&render_ctx.audio_clock);

      if (IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_Q))
      {
        StopMusicStream(music);
        PlayMusicStream(music);
        PauseMusicStream(music);
        audio_clock_start(&render_ctx.audio_clock);
        audio_clock_pause(&render_ctx.audio_clock);
      }
      else if (IsKeyPressed(KEY_Q))
      {
        if (render_ctx.audio_clock.is_playing) { PauseMusicStream(music) ; audio_clock_pause(&render_ctx.audio_clock) ; }
        else                                   { ResumeMusicStream(music); audio_clock_resume(&render_ctx.audio_clock); }
      }
    }

    // Reload config.ini
    if ((IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER)) &&
        (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) &&
        IsKeyPressed(KEY_R))
    {
      app_configs = app_configs_init();
      app_configs.should_recalibrate_camera = true;
      SetMusicVolume(music, app_configs.music_volume);
      selection_service_unload(&selection_service);
    }

    // KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA
    if ((IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER)) &&
        (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) &&
        IsKeyPressed(KEY_K))
      app_configs.kohra = !app_configs.kohra;
    // KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA KOHRA

    BeginTextureMode(target);
      rlMatrixMode(RL_PROJECTION);
      rlLoadIdentity();
      rlOrtho(0, GetScreenWidth(), GetScreenHeight(), 0, 0.0f, 1.0f);
      rlMatrixMode(RL_MODELVIEW);
      rlLoadIdentity();
      ClearBackground(WHITE);

      // Gameplay scene
      // Order: Base track -> Notes -> Sky Input/Label -> HUD -> Windows
      track_service_render_base_track(&track_service, &render_ctx, &chart_reader.render_lists);
      notes_service_render(&notes_service, &render_ctx, &chart_reader, &selection_service.raycast_results);
      track_service_render_sky_input(&track_service, &render_ctx, &chart_reader.render_lists);
      hud_services_render(&hud_service, &render_ctx);

    EndTextureMode();

    BeginDrawing();
      ClearBackground(WHITE);

      float timing = (audio_clock_get_time_ms(&render_ctx.audio_clock) - render_ctx.chart_settings.audio_offset);
      float screen_size[2] = {(float)GetRenderWidth(), (float)GetRenderHeight()};
      SetShaderValue(chart_shader, timing_loc, &timing, SHADER_UNIFORM_FLOAT);
      SetShaderValue(chart_shader, screenSize_loc, &screen_size, SHADER_UNIFORM_VEC2);

      BeginShaderMode(chart_shader);
        DrawTexturePro(target.texture,
                       (Rectangle){ 0, 0, (float)target.texture.width, (float)-target.texture.height },
                       (Rectangle){ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() },
                       Vector2Zero(), 0.0f, WHITE);
      EndShaderMode();

      // Windows
      cmd_palette_draw (&window_service.command_palette, (CmdPltData *)window_service.command_palette.data);
      proj_setting_draw(&window_service.project_setting, (ProjSettingData *)window_service.project_setting.data,
                        &app_configs, &chart_reader.timing_groups, &font_probes);

      // Debug FPS
      DrawFPS(5, 5);

      GuiWindowFileDialog(&open_proj_input.state);

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

    // Temp place to update AppConfigs values to other places if needed
    {
      if(app_configs.colorblind != notes_service.colorblind)
        notes_service.colorblind = app_configs.colorblind;
      if (app_configs.playfield_ratio != hud_service.playfield_ratio)
        hud_service.playfield_ratio = app_configs.playfield_ratio;
      if(chart_reader.initialized)
      {
        if(fabsf(app_configs.scroll_speed - render_ctx.chart_settings.scroll_speed) > 1e-6)
        {
          ChartSettings *chart_settings = &render_ctx.chart_settings;
          chart_settings->scroll_speed  = app_configs.scroll_speed;
          chart_reader.low_z_clip       = z_to_floor_position(   9.0, chart_settings->base_bpm, chart_settings->scroll_speed);
          chart_reader.high_z_clip      = z_to_floor_position(-100.0, chart_settings->base_bpm, chart_settings->scroll_speed);
        }
      }
      if(app_configs.should_recalibrate_camera)
      {
        SetWindowSize(GetScreenWidth(), (int)aspect_ratio_get_height((float)GetScreenWidth(), app_configs.playfield_ratio));
        recalibrate_camera(&render_ctx.camera);
        app_configs.should_recalibrate_camera = false;
      }
      if(app_configs.should_apply_screen_scale)
      {
        SetWindowSize(app_configs.screen_width, (int)aspect_ratio_get_height((float)app_configs.screen_width, app_configs.playfield_ratio));
        recalibrate_camera(&render_ctx.camera);
        app_configs.should_apply_screen_scale = false;
      }
    }

    if(open_proj_input.state.SelectFilePressed)
    {
      bool is_valid = false;
      if (open_proj_input.extensions.size == 0) is_valid = true;
      else
      {
        for (int i = 0; i < open_proj_input.extensions.size; i++)
        {
          const char* ext = (char *)list_get(&open_proj_input.extensions, i);
          is_valid = IsFileExtension(open_proj_input.state.fileNameText, ext);
          if (is_valid) break;
        }
      }
      if(is_valid)
        strcpy(open_proj_input.file, TextFormat("%s%s%s", open_proj_input.state.dirPathText, PATH_SEPERATOR, open_proj_input.state.fileNameText));

      const char* project_file = open_proj_input.file;
      process_imported_project(project_file, &music,
                               &app_configs, &font_probes,
                               &track_service, &notes_service, &hud_service, &window_service,
                               &chart_reader, &render_ctx, &selection_service,
                               &chart_shader, &timing_loc, &screenSize_loc);
      open_proj_input.state.SelectFilePressed = false;
    }
  }

  rgui_fileinput_unload(&open_proj_input);

  StopMusicStream(music);
  UnloadMusicStream(music);
  CloseAudioDevice();

  // Unload stuffs
  chart_reader_unload(&chart_reader);
  track_service_unload(&track_service);
  notes_service_unload(&notes_service);
  hud_service_unload(&hud_service);
  textures_unload(&texture_group);
  windows_services_unload(&window_service);
  cached_font_probes_unload(&font_probes);
  selection_service_unload(&selection_service);
  UnloadShader(chart_shader);

  // One last config writing just in case
  app_configs_write_to_file(&app_configs);
  app_configs_unload(&app_configs);

  // destroy the window and cleanup the OpenGL context
  CloseWindow();
  return 0;
}
