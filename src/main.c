/*
Cloned from quickstart `https://github.com/raylib-extras/raylib-quickstart` by Jeffery Myers, marked with CC0 1.0.
To view a copy of this license, visit https://creativecommons.org/publicdomain/zero/1.0/
*/

#include "data/app_configs/app_config.h"
#define RINI_IMPLEMENTATION
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "rlgl.h"

#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "raylib.h"
#include "cJSON.h"
#include "constants.h"
#include "resource_util.h"
#include "data/chart_settings/chart_settings.h"
#include "gameplay/audio_service.h"
#include "gameplay/camera/camera_service.h"
#include "gameplay/chart_reader.h"
#include "gameplay/hud/hud_services.h"
#include "render/playfield/playfield_services.h"
#include "render/render_service.h"
#include "render/texture/texture_service.h"
#include "render/texture/skin_side.h"
#include "render/texture/single_line_type.h"
#include "render/mesh_renderable.h"
#include "windows/window_inst.h"
#include "windows/window_services.h"

#define GLSL_VERSION 330

int main()
{
  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
  InitWindow(1280, 720, "arkohra");
  SetExitKey(KEY_NULL);

  rini_data rini_d              = fetch_rini_config();
  AppConfigs app_configs        = app_configs_init(&rini_d);
  Camera3D camera               = camera_init_playfield();
  TextureGroup texture_group    = textures_init();
  ChartSettings chart_settings  = chart_settings_init(&app_configs);
  ChartReader chart_reader      = { 0 };

  WindowGroup window_group      = window_services_init();
  PlayfieldObjs playfield_objs  = playfield_objs_init(&texture_group);

  FontServices font_services    = font_services_init(GLSL_VERSION);
  Shader arc_clip_shader        = LoadShader(
      TextFormat("resources/shaders/glsl%i/arc_clip.vs", GLSL_VERSION),
      // 0,
      TextFormat("resources/shaders/glsl%i/arc_clip.fs", GLSL_VERSION)
  );
  int isVoid_loc = GetShaderLocation(arc_clip_shader, "isVoid");
  int shouldClip_loc = GetShaderLocation(arc_clip_shader, "shouldClip");
  int negativeBPM_loc = GetShaderLocation(arc_clip_shader, "negativeBPM");
  printf("negativeBPM: %d\n", negativeBPM_loc);
  printf("mvp loc: %d\n", arc_clip_shader.locs[SHADER_LOC_MATRIX_MVP]);
  printf("matModel loc: %d\n", arc_clip_shader.locs[SHADER_LOC_MATRIX_MODEL]);

  RenderContext render_ctx = {
    .camera          = camera,
    .chart_settings  = chart_settings,
    .arc_clip_shader = {
      .shader = arc_clip_shader,
      .isVoid_loc = isVoid_loc,
      .shouldClip_loc = shouldClip_loc,
      .negativeBPM_loc = negativeBPM_loc
    },
    .audio_clock     = { 0 },
  };


  InitAudioDevice();
  Wave wave = LoadWave(render_ctx.chart_settings.audio_path);
  float wave_time_ms = ((float)wave.frameCount * 1000) / (float)(wave.sampleRate);

  Music music = LoadMusicStream("");
  music.looping = false;
  bool pause = true;
  SetMusicPan(music, 0.0f);
  SetMusicVolume(music, app_configs.music_volume);
  PlayMusicStream(music);
  PauseMusicStream(music);
  audio_clock_pause(&render_ctx.audio_clock);

  bool has_kohra = false;
  float current_ms = 0;

  SetTargetFPS(60);
  rlSetClipPlanes(0.01f, 100.0f);

  while (!WindowShouldClose())
  {
    if (IsFileDropped())
    {
      // Pause audio before processing
      pause = true;
      PauseMusicStream(music);
      audio_clock_pause(&render_ctx.audio_clock);

      // Parse and repolulate chart settings
      bool status = false;
      FilePathList dropped_file = LoadDroppedFiles();
      char *project_file = dropped_file.paths[0];
      const char *dir = GetDirectoryPath(project_file);
      if (TextIsEqual(GetFileName(project_file), "project.json"))
      {
        const cJSON *last_opened_chart_path = NULL;
        const cJSON *charts                 = NULL;
        const cJSON *chart_item             = NULL;
        char *project_json_text = LoadFileText(project_file);
        cJSON *json = cJSON_Parse(project_json_text);
        if (json == NULL)
        {
          const char *error_ptr = cJSON_GetErrorPtr();
          if (error_ptr != NULL)
          {
            fprintf(stderr, "Error before %s\n", error_ptr);
            goto parsing_project_end;
          }
        }

        last_opened_chart_path = cJSON_GetObjectItemCaseSensitive(json, "lastOpenedChartPath");
        char aff_file_search[16]; TextCopy(aff_file_search, "2.aff");
        if (cJSON_IsString(last_opened_chart_path) && last_opened_chart_path->valuestring != NULL)
          TextCopy(aff_file_search, last_opened_chart_path->valuestring);

        charts = cJSON_GetObjectItemCaseSensitive(json, "charts");
        cJSON_ArrayForEach(chart_item, charts)
        {
          // Check for matching last opened chart path
          cJSON *chart_path = cJSON_GetObjectItemCaseSensitive(chart_item, "chartPath");
          if (!cJSON_IsString(chart_path) || !TextIsEqual(aff_file_search, chart_path->valuestring))
            continue;

          // Require audio path to continue parsing
          cJSON *audio_path = cJSON_GetObjectItemCaseSensitive(chart_item, "audioPath");
          if (!cJSON_IsString(audio_path)) goto parsing_project_end;

          // Reset chart settings
          render_ctx.chart_settings = chart_settings_init(&app_configs);

          render_ctx.chart_settings.scroll_speed = app_configs.scroll_speed;
          TextCopy(render_ctx.chart_settings.chart_path, TextFormat("%s/%s", dir, chart_path->valuestring));
          TextCopy(render_ctx.chart_settings.audio_path, TextFormat("%s/%s", dir, audio_path->valuestring));

          cJSON *jacket_path = cJSON_GetObjectItemCaseSensitive(chart_item, "jacketPath");
          if (cJSON_IsString(jacket_path) && jacket_path->valuestring != NULL)
            TextCopy(render_ctx.chart_settings.jacket_path, TextFormat("%s/%s", dir, jacket_path->valuestring));

          cJSON *background_path = cJSON_GetObjectItemCaseSensitive(chart_item, "backgroundPath");
          if (cJSON_IsString(background_path) && background_path->valuestring != NULL)
            TextCopy(render_ctx.chart_settings.background_path, TextFormat("%s/%s", dir, background_path->valuestring));

          cJSON *base_bpm = cJSON_GetObjectItemCaseSensitive(chart_item, "baseBpm");
          if (cJSON_IsNumber(base_bpm)) render_ctx.chart_settings.base_bpm = (float)base_bpm->valuedouble;

          cJSON *bpm_text = cJSON_GetObjectItemCaseSensitive(chart_item, "bpmText");
          if (cJSON_IsString(bpm_text) && bpm_text->valuestring != NULL)
            TextCopy(render_ctx.chart_settings.bpm_text, bpm_text->valuestring);

          cJSON *sync_base_bpm = cJSON_GetObjectItemCaseSensitive(chart_item, "syncBaseBpm");
          if (cJSON_IsBool(sync_base_bpm)) render_ctx.chart_settings.sync_base_bpm = cJSON_IsTrue(sync_base_bpm);

          cJSON *title = cJSON_GetObjectItemCaseSensitive(chart_item, "title");
          if (cJSON_IsString(title) && title->valuestring != NULL)
            TextCopy(render_ctx.chart_settings.title, title->valuestring);

          cJSON *composer = cJSON_GetObjectItemCaseSensitive(chart_item, "composer");
          if (cJSON_IsString(composer) && composer->valuestring != NULL)
            TextCopy(render_ctx.chart_settings.composer, composer->valuestring);

          cJSON *alias = cJSON_GetObjectItemCaseSensitive(chart_item, "alias");
          if (cJSON_IsString(alias) && alias->valuestring != NULL)
            TextCopy(render_ctx.chart_settings.alias, alias->valuestring);

          cJSON *charter = cJSON_GetObjectItemCaseSensitive(chart_item, "charter");
          if (cJSON_IsString(charter) && charter->valuestring != NULL)
            TextCopy(render_ctx.chart_settings.charter, charter->valuestring);

          cJSON *illustrator = cJSON_GetObjectItemCaseSensitive(chart_item, "illustrator");
          if (cJSON_IsString(illustrator) && illustrator->valuestring != NULL)
            TextCopy(render_ctx.chart_settings.illustrator, illustrator->valuestring);

          cJSON *difficulty = cJSON_GetObjectItemCaseSensitive(chart_item, "difficulty");
          if (cJSON_IsString(difficulty) && difficulty->valuestring != NULL)
            TextCopy(render_ctx.chart_settings.difficulty, difficulty->valuestring);

          cJSON *chart_constant = cJSON_GetObjectItemCaseSensitive(chart_item, "chartConstant");
          if (cJSON_IsNumber(chart_constant)) render_ctx.chart_settings.chart_constant = (float)chart_constant->valuedouble;

          cJSON *difficulty_color = cJSON_GetObjectItemCaseSensitive(chart_item, "difficultyColor");
          if (cJSON_IsString(difficulty_color) && difficulty_color->valuestring != NULL)
            TextCopy(render_ctx.chart_settings.difficulty_color, difficulty_color->valuestring);

          cJSON *skin = cJSON_GetObjectItemCaseSensitive(chart_item, "skin");
          if (cJSON_IsObject(skin)){
            cJSON *side = cJSON_GetObjectItemCaseSensitive(skin, "side");
            if (cJSON_IsString(side) && side->valuestring != NULL)
            {
              render_ctx.chart_settings.skin_track = skin_side_get_by_string(side->valuestring);
              render_ctx.chart_settings.skin_side  = skin_side_get_by_string(side->valuestring);
            }

            cJSON *track = cJSON_GetObjectItemCaseSensitive(skin, "track");
            if (cJSON_IsString(track) && track->valuestring != NULL)
              render_ctx.chart_settings.skin_track = skin_side_get_by_string(track->valuestring);

            cJSON *single_line= cJSON_GetObjectItemCaseSensitive(skin, "singleLine");
            if (cJSON_IsString(single_line) && single_line->valuestring != NULL)
              render_ctx.chart_settings.sl_type = single_line_get_by_string(single_line->valuestring);
          }

          status = true;
        }

        parsing_project_end:
        cJSON_Delete(json);
        UnloadDroppedFiles(dropped_file);

        if (status)
        {
          // Update chart reader
          if(chart_reader.initialized) chart_reader_unload(&chart_reader);
          chart_reader = chart_reader_parse((char *)&render_ctx.chart_settings.chart_path, &render_ctx, &texture_group.arc);
          chart_settings_print(&render_ctx.chart_settings);

          // Update texture group
          textures_unload(&texture_group);
          texture_group = textures_init();
          if (!TextIsEqual(render_ctx.chart_settings.jacket_path, ""))
          {
            UnloadTexture(texture_group.jacket_img);
            texture_group.jacket_img = LoadTexture(render_ctx.chart_settings.jacket_path);
            SetTextureFilter(texture_group.jacket_img, TEXTURE_FILTER_BILINEAR);
          }
          if (!TextIsEqual(render_ctx.chart_settings.background_path, ""))
          {
            UnloadTexture(texture_group.background);
            texture_group.background = LoadTexture(render_ctx.chart_settings.background_path);
            SetTextureFilter(texture_group.background, TEXTURE_FILTER_BILINEAR);
          }

          UnloadTexture(texture_group.track);
          skin_side_load_track(render_ctx.chart_settings.skin_track, &texture_group);
          SetTextureWrap(texture_group.track, TEXTURE_WRAP_REPEAT);

          UnloadTexture(texture_group.hold);
          skin_side_load_hold(render_ctx.chart_settings.skin_side, &texture_group);
          SetTextureFilter(texture_group.hold, TEXTURE_FILTER_BILINEAR);

          UnloadTexture(texture_group.tap);
          skin_side_load_tap(render_ctx.chart_settings.skin_side, &texture_group);
          SetTextureFilter(texture_group.tap, TEXTURE_FILTER_BILINEAR);

          UnloadTexture(texture_group.arctap);
          skin_side_load_arctap(render_ctx.chart_settings.skin_side, &texture_group);
          SetTextureFilter(texture_group.arctap, TEXTURE_FILTER_BILINEAR);

          UnloadTexture(texture_group.single_line);
          single_line_load(render_ctx.chart_settings.sl_type, &texture_group);
          SetTextureWrap(texture_group.single_line, TEXTURE_WRAP_REPEAT);

          List hud_code_points; list_init(&hud_code_points, sizeof(int));
          for (int cp = 0x20; cp <= 0x7E; cp++) list_push(&hud_code_points, &cp);
          AddStringToCodepointList(&hud_code_points, render_ctx.chart_settings.title);
          AddStringToCodepointList(&hud_code_points, render_ctx.chart_settings.composer);
          AddStringToCodepointList(&hud_code_points, render_ctx.chart_settings.difficulty);
          font_services.hud_notosans_tc_reg = GenerateSDF((char *)"resources/fonts/NotoSansTC-Regular.ttf", 45, (int *)hud_code_points.data, hud_code_points.size);
          list_free(&hud_code_points);

          // Update music stream + audio clock
          pause = true;
          StopMusicStream(music);
          UnloadMusicStream(music);
          music = LoadMusicStream(render_ctx.chart_settings.audio_path);
          music.looping = false;
          SetMusicPan(music, 0.0f);
          SetMusicVolume(music, app_configs.music_volume);
          PlayMusicStream(music);
          PauseMusicStream(music);
          audio_clock_start(&render_ctx.audio_clock);
          audio_clock_pause(&render_ctx.audio_clock);
        }
      }
    }

    if (IsWindowResized()) recalibrate_camera(&render_ctx.camera);

    if (!pause)
    {
      // Playfield: Track & Single Line Scrolling
      static float scroll_offset = 0.0f;
      scroll_offset += GetFrameTime() * render_ctx.chart_settings.scroll_speed;
      renderable_update_scroll(&playfield_objs.track      , scroll_offset);
      renderable_update_scroll(&playfield_objs.single_line, scroll_offset);
    }

    if (IsMusicValid(music))
    {
      UpdateMusicStream(music);
      current_ms = audio_clock_get_time_ms(&render_ctx.audio_clock);

      if (!IsMusicStreamPlaying(music) && render_ctx.audio_clock.is_playing)
      {
        pause = true;
        audio_clock_pause(&render_ctx.audio_clock);
      }

      if (IsKeyPressed(KEY_Q))
      {
        // Song reached to the end naturally (without manually pausing)
        if (!pause && !IsMusicStreamPlaying(music))
        {
          pause = false;
          StopMusicStream(music);
          PlayMusicStream(music);
          audio_clock_start(&render_ctx.audio_clock);
        }
        else
        {
          pause = !pause;
          if (pause) { PauseMusicStream(music) ; audio_clock_pause(&render_ctx.audio_clock) ; }
          else       { ResumeMusicStream(music); audio_clock_resume(&render_ctx.audio_clock); }
        }
      }
      if (IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_Q))
      {
        pause = true;
        StopMusicStream(music);
        PlayMusicStream(music);
        PauseMusicStream(music);
        audio_clock_start(&render_ctx.audio_clock);
        audio_clock_pause(&render_ctx.audio_clock);
      }
    }

    // Windows keybinds
    if ((IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER)) &&
        (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) &&
        IsKeyPressed(KEY_P))
      window_inst_toggle(&window_group.command_palette);
    if (IsKeyPressed(KEY_ESCAPE))
      window_inst_close(&window_group.command_palette);

    // Kohra keybind
    if ((IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER)) &&
        (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) &&
        IsKeyPressed(KEY_K))
      has_kohra = !has_kohra;

    windows_services_ui_update(&window_group);

    BeginDrawing();
      ClearBackground(WHITE);

      playfield_render(&render_ctx, &chart_reader, &texture_group, &playfield_objs, current_ms - render_ctx.chart_settings.audio_offset);
      hud_services_render(&texture_group, &render_ctx.chart_settings, &font_services);
      windows_services_render(&window_group);

      // Debug FPS
      DrawFPS(5, 5); 

      if (has_kohra)
      {
        BeginMode3D(render_ctx.camera);
          DrawCubeTexture(texture_group.kohra, (Vector3){0.0f, KOHRA_SIZE, 0.0f}, KOHRA_SIZE, KOHRA_SIZE, KOHRA_SIZE, WHITE);
        EndMode3D();
      }

    // end the frame and get ready for the next one  (display frame, poll input, etc...)
    EndDrawing();
  }

  // Unload stuffs
  UnloadMusicStream(music);
  UnloadWave(wave);
  CloseAudioDevice();

  chart_reader_unload(&chart_reader);
  playfield_objs_unload(&playfield_objs);
  textures_unload(&texture_group);
  windows_services_unload(&window_group);
  font_services_unload(&font_services);

  // One last config writing just in case
  app_configs_write_to_file(&app_configs, &rini_d);

  // destroy the window and cleanup the OpenGL context
  CloseWindow();
  return 0;
}
