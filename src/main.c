/*
Cloned from quickstart `https://github.com/raylib-extras/raylib-quickstart` by Jeffery Myers, marked with CC0 1.0.
To view a copy of this license, visit https://creativecommons.org/publicdomain/zero/1.0/
*/

#include "data/app_configs/app_config.h"

#define RINI_IMPLEMENTATION
#include "raylib.h"
#include "raygui.h"
#include "rlgl.h"

#include <stdlib.h>
#include "cJSON.h"
#include "constants.h"
#include "resource_util.h"
#include "data/chart_settings/chart_settings.h"
#include "data/custom_types/dynamic_list.h"
#include "data/fonts/fonts_service.h"
#include "gameplay/audio_service.h"
#include "gameplay/camera/camera_service.h"
#include "gameplay/chart_reader.h"
#include "render/render_service.h"
#include "render/texture/texture_service.h"
#include "render/texture/skin_side.h"
#include "render/texture/single_line_type.h"
#include "render/hud/hud_services.h"
#include "render/notes/notes_service.h"
#include "render/track/track_service.h"
#include "render/utils/drawing.h"
#include "windows/window_inst.h"
#include "windows/window_services.h"

#define GLSL_VERSION 330

int main()
{
  rini_data rini_d           = fetch_rini_config();
  AppConfigs app_configs     = app_configs_init(&rini_d);

  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(BASE_APP_WINDOW_WIDTH,
             aspect_ratio_get_height((float)BASE_APP_WINDOW_WIDTH, app_configs.playfield_ratio),
             "arkohra");
  SetExitKey(KEY_NULL);

  TextureGroup texture_group = textures_init();
  ChartReader chart_reader   = { 0 };
  WindowGroup window_group   = window_services_init(GLSL_VERSION);

  RenderContext render_ctx = {
    .camera         = camera_init_playfield(),
    .chart_settings = chart_settings_init(&app_configs),
    .audio_clock    = { 0 },
    .track_service  = track_service_init(),
    .notes_service  = notes_service_init(GLSL_VERSION),
    .hud_service    = hud_service_init(GLSL_VERSION)
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
          text_copy_bounded(aff_file_search, 16, last_opened_chart_path->valuestring);

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
          ChartSettings *chart_settings = &render_ctx.chart_settings;

          text_copy_bounded(chart_settings->chart_path, MAXPATHLEN, TextFormat("%s/%s", dir, chart_path->valuestring));
          text_copy_bounded(chart_settings->audio_path, MAXPATHLEN, TextFormat("%s/%s", dir, audio_path->valuestring));

          cJSON *jacket_path = cJSON_GetObjectItemCaseSensitive(chart_item, "jacketPath");
          if (cJSON_IsString(jacket_path) && jacket_path->valuestring != NULL)
            text_copy_bounded(chart_settings->jacket_path, MAXPATHLEN, TextFormat("%s/%s", dir, jacket_path->valuestring));

          cJSON *background_path = cJSON_GetObjectItemCaseSensitive(chart_item, "backgroundPath");
          if (cJSON_IsString(background_path) && background_path->valuestring != NULL)
            text_copy_bounded(chart_settings->background_path, MAXPATHLEN, TextFormat("%s/%s", dir, background_path->valuestring));

          cJSON *base_bpm = cJSON_GetObjectItemCaseSensitive(chart_item, "baseBpm");
          if (cJSON_IsNumber(base_bpm)) chart_settings->base_bpm = (float)base_bpm->valuedouble;

          cJSON *bpm_text = cJSON_GetObjectItemCaseSensitive(chart_item, "bpmText");
          if (cJSON_IsString(bpm_text) && bpm_text->valuestring != NULL)
            text_copy_bounded(chart_settings->bpm_text, 256, bpm_text->valuestring);

          cJSON *sync_base_bpm = cJSON_GetObjectItemCaseSensitive(chart_item, "syncBaseBpm");
          if (cJSON_IsBool(sync_base_bpm)) chart_settings->sync_base_bpm = cJSON_IsTrue(sync_base_bpm);

          cJSON *title = cJSON_GetObjectItemCaseSensitive(chart_item, "title");
          if (cJSON_IsString(title) && title->valuestring != NULL)
            text_copy_bounded(chart_settings->title, 256, title->valuestring);

          cJSON *composer = cJSON_GetObjectItemCaseSensitive(chart_item, "composer");
          if (cJSON_IsString(composer) && composer->valuestring != NULL)
            text_copy_bounded(chart_settings->composer, 256, composer->valuestring);

          cJSON *alias = cJSON_GetObjectItemCaseSensitive(chart_item, "alias");
          if (cJSON_IsString(alias) && alias->valuestring != NULL)
            text_copy_bounded(chart_settings->alias, 256, alias->valuestring);

          cJSON *charter = cJSON_GetObjectItemCaseSensitive(chart_item, "charter");
          if (cJSON_IsString(charter) && charter->valuestring != NULL)
            text_copy_bounded(chart_settings->charter, 256, charter->valuestring);

          cJSON *illustrator = cJSON_GetObjectItemCaseSensitive(chart_item, "illustrator");
          if (cJSON_IsString(illustrator) && illustrator->valuestring != NULL)
            text_copy_bounded(chart_settings->illustrator, 256, illustrator->valuestring);

          cJSON *difficulty = cJSON_GetObjectItemCaseSensitive(chart_item, "difficulty");
          if (cJSON_IsString(difficulty) && difficulty->valuestring != NULL)
            text_copy_bounded(chart_settings->difficulty, 256, difficulty->valuestring);

          cJSON *chart_constant = cJSON_GetObjectItemCaseSensitive(chart_item, "chartConstant");
          if (cJSON_IsNumber(chart_constant)) chart_settings->chart_constant = (float)chart_constant->valuedouble;

          cJSON *difficulty_color = cJSON_GetObjectItemCaseSensitive(chart_item, "difficultyColor");
          if (cJSON_IsString(difficulty_color) && difficulty_color->valuestring != NULL)
            text_copy_bounded(chart_settings->difficulty_color, 256, difficulty_color->valuestring);

          cJSON *skin = cJSON_GetObjectItemCaseSensitive(chart_item, "skin");
          if (cJSON_IsObject(skin)){
            cJSON *side = cJSON_GetObjectItemCaseSensitive(skin, "side");
            if (cJSON_IsString(side) && side->valuestring != NULL)
            {
              chart_settings->skin_track = skin_side_get_by_string(side->valuestring);
              chart_settings->skin_side  = skin_side_get_by_string(side->valuestring);
            }

            cJSON *track = cJSON_GetObjectItemCaseSensitive(skin, "track");
            if (cJSON_IsString(track) && track->valuestring != NULL)
              chart_settings->skin_track = skin_side_get_by_string(track->valuestring);

            cJSON *single_line= cJSON_GetObjectItemCaseSensitive(skin, "singleLine");
            if (cJSON_IsString(single_line) && single_line->valuestring != NULL)
              chart_settings->sl_type = single_line_get_by_string(single_line->valuestring);
          }

          status = true;
        }

        parsing_project_end:
        cJSON_Delete(json);

        if (status)
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

          chart_reader = chart_reader_parse((char *)&render_ctx.chart_settings.chart_path,
                                            &render_ctx.chart_settings,
                                            &render_ctx.audio_clock,
                                            &render_ctx.notes_service.arc_tex,
                                            &render_ctx.notes_service.arc_shader.shader);
          chart_settings_print(&render_ctx.chart_settings);

          // Update texture group
          ChartSettings *chart_settings = &render_ctx.chart_settings;
          TrackService *track_service = &render_ctx.track_service;
          NotesService *notes_service = &render_ctx.notes_service;
          HudService *hud_service = &render_ctx.hud_service;

          UnloadTexture(hud_service->jacket_img);
          if (!TextIsEqual(chart_settings->jacket_path, ""))
          {
            hud_service->jacket_img = LoadTexture(chart_settings->jacket_path);
            if (!IsTextureValid(hud_service->jacket_img))
              hud_service->jacket_img = LoadTexture("resources/gameplay/DefaultJacket.png");
          } else hud_service->jacket_img = LoadTexture("resources/gameplay/DefaultJacket.png");
          SetTextureFilter(hud_service->jacket_img, TEXTURE_FILTER_BILINEAR);

          UnloadTexture(track_service->background_tex);
          if (!TextIsEqual(chart_settings->background_path, ""))
          {
            track_service->background_tex = LoadTexture(chart_settings->background_path);
            if (!IsTextureValid(track_service->background_tex))
              track_service->background_tex = LoadTexture("resources/gameplay/DefaultBackgrounds/arccreate-blender2_base_light.jpg");
          } else track_service->background_tex = LoadTexture("resources/gameplay/DefaultBackgrounds/arccreate-blender2_base_light.jpg");
          SetTextureFilter(track_service->background_tex, TEXTURE_FILTER_BILINEAR);

          UnloadTexture(track_service->track_tex);
          track_service->track_tex = skin_side_get_track(chart_settings->skin_track);
          SetTextureWrap(track_service->track_tex, TEXTURE_WRAP_REPEAT);

          UnloadTexture(notes_service->hold_tex);
          notes_service->hold_tex = skin_side_get_hold(chart_settings->skin_side);
          SetTextureFilter(render_ctx.notes_service.hold_tex, TEXTURE_FILTER_BILINEAR);

          UnloadTexture(notes_service->tap_tex);
          notes_service->tap_tex = skin_side_get_tap(chart_settings->skin_side);
          SetTextureFilter(notes_service->tap_tex, TEXTURE_FILTER_BILINEAR);

          UnloadTexture(notes_service->arctap_tex);
          notes_service->arctap_tex = skin_side_get_arctap(chart_settings->skin_side);
          SetTextureFilter(notes_service->arctap_tex, TEXTURE_FILTER_BILINEAR);

          UnloadTexture(track_service->single_line_tex);
          track_service->single_line_tex = single_line_get(chart_settings->sl_type);
          SetTextureWrap(track_service->single_line_tex, TEXTURE_WRAP_REPEAT);

          const char *paths[] = {
            "resources/fonts/NotoSans-Regular.ttf",
            "resources/fonts/NotoSansSC-Regular.ttf",
            "resources/fonts/NotoSansJP-Regular.ttf",
            "resources/fonts/NotoSansKR-Regular.ttf",
            "resources/fonts/NotoSansMath-Regular.ttf",
          };
          List font_list; list_init(&font_list, sizeof(char *));
          for (int i = 0; i < 5; i++) list_push(&font_list, &paths[i]);
          List hud_code_points; list_init(&hud_code_points, sizeof(int));
          for (int cp = 0x20; cp <= 0x7E; cp++) list_push(&hud_code_points, &cp);
          font_add_string_to_codepoints(&hud_code_points, chart_settings->title);
          font_add_string_to_codepoints(&hud_code_points, chart_settings->composer);
          font_add_string_to_codepoints(&hud_code_points, chart_settings->difficulty);
          for (int i = 0; i < hud_service->font_with_fallback.size; i++)
          {
            Font *font = (Font *)list_get(&hud_service->font_with_fallback, i);
            UnloadFont(*font);
          }
          list_clear(&hud_service->font_with_fallback);
          hud_service->font_with_fallback = fonts_init(&font_list, 45, (int *)hud_code_points.data, hud_code_points.size);
          list_free(&hud_code_points);
          list_free(&font_list);

          PlayMusicStream(music);
          PauseMusicStream(music);
          audio_clock_start(&render_ctx.audio_clock);
          audio_clock_pause(&render_ctx.audio_clock);
        }
      }
      UnloadDroppedFiles(dropped_file);
    }
    if (IsWindowResized())
    {
      SetWindowSize(GetScreenWidth(), (int)aspect_ratio_get_height((float)GetScreenWidth(), app_configs.playfield_ratio));
      recalibrate_camera(&render_ctx.camera);
    }

    // TODO: currently scrolling with constant speed, find a way to speed up/slow down based on first timing group's current bpm
    if (render_ctx.audio_clock.is_playing)
    {
      // Playfield: Track & Single Line Scrolling
      static float scroll_offset = 0.0f;
      scroll_offset += GetFrameTime() * render_ctx.chart_settings.scroll_speed;
      renderable_update_scroll(&render_ctx.track_service.track      , scroll_offset);
      renderable_update_scroll(&render_ctx.track_service.single_line, scroll_offset);
    }

    if (IsMusicValid(music))
    {
      UpdateMusicStream(music);

      if (!IsMusicStreamPlaying(music) && render_ctx.audio_clock.is_playing)
        audio_clock_pause(&render_ctx.audio_clock);

      if (IsKeyPressed(KEY_Q))
      {
        // Song reached to the end naturally (without manually pausing)
        if (render_ctx.audio_clock.is_playing && !IsMusicStreamPlaying(music))
        {
          StopMusicStream(music);
          PlayMusicStream(music);
          audio_clock_start(&render_ctx.audio_clock);
        }
        else
        {
          if (render_ctx.audio_clock.is_playing) { PauseMusicStream(music) ; audio_clock_pause(&render_ctx.audio_clock) ; }
          else                                   { ResumeMusicStream(music); audio_clock_resume(&render_ctx.audio_clock); }
        }
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

    // Windows keybinds
    if ((IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER)) &&
        (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) &&
        IsKeyPressed(KEY_P))
      window_inst_toggle(&window_group.command_palette);
    if (IsKeyPressed(KEY_ESCAPE))
      window_inst_close(&window_group.command_palette);

    if ((IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER)) &&
        IsKeyPressed(KEY_COMMA))
    {
      window_inst_close(&window_group.command_palette);
      window_inst_toggle(&window_group.project_setting);
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

      // Update chart reader to apply scroll speed
      if(chart_reader.initialized) chart_reader_unload(&chart_reader);
      chart_reader = chart_reader_parse((char *)&render_ctx.chart_settings.chart_path,
                                        &render_ctx.chart_settings,
                                        &render_ctx.audio_clock,
                                        &render_ctx.notes_service.arc_tex,
                                        &render_ctx.notes_service.arc_shader.shader);
    }

    // Kohra keybind
    if ((IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER)) &&
        (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) &&
        IsKeyPressed(KEY_K))
      app_configs.kohra = !app_configs.kohra;

    windows_services_ui_update(&window_group);

    BeginDrawing();
      ClearBackground(WHITE);

      render_scenes(&render_ctx, &chart_reader);
      windows_services_render(&window_group);

      // Debug FPS
      DrawFPS(5, 5);

      if (app_configs.kohra)
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
  textures_unload(&texture_group);
  windows_services_unload(&window_group);
  render_unload(&render_ctx);

  // One last config writing just in case
  app_configs_write_to_file(&app_configs, &rini_d);

  // destroy the window and cleanup the OpenGL context
  CloseWindow();
  return 0;
}
