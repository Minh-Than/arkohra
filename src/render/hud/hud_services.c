#include <math.h>
#include "data/app_configs/app_config.h"
#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include "color_services.h"
#include "constants.h"
#include "hud_services.h"
#include "data/chart_settings/chart_settings.h"
#include "data/custom_types/dynamic_list.h"
#include "data/fonts/fonts_service.h"
#include "gameplay/audio_service.h"

HudService hud_service_init(int glsl, AppConfigs *app_configs, CachedFontProbes *font_probes)
{
  HudService service = { 0 };
  service.sdf_shader    = LoadShader(0, TextFormat("resources/shaders/glsl%i/sdf.fs", glsl));
  service.saira_regular = font_generate_sdf(font_probes->saira_reg.data, font_probes->saira_reg.size, 45, NULL, 95);
  service.saira_medium = font_generate_sdf(font_probes->saira_med.data, font_probes->saira_med.size, 45, NULL, 95);

  List fallback_probes; list_init(&fallback_probes, sizeof(FontProbe));
  list_push(&fallback_probes, &font_probes->noto_reg);

  List hud_code_points; list_init(&hud_code_points, sizeof(int));
  for (int cp = 0x20; cp <= 0x7E; cp++) list_push(&hud_code_points, &cp);
  service.font_with_fallback = fonts_init(&fallback_probes, 45, (int *)hud_code_points.data, hud_code_points.size);
  list_free(&hud_code_points);
  list_free(&fallback_probes);

  service.pause_button  = LoadTexture("resources/gameplay/HUD/PauseLight.png");
  service.info_panel    = LoadTexture("resources/gameplay/HUD/InfoLight.png");
  service.jacket_bg     = LoadTexture("resources/gameplay/HUD/JacketBackground.png");
  service.jacket_img    = LoadTexture("resources/gameplay/DefaultJacket.png");
  service.jacket_diff   = LoadTexture("resources/gameplay/HUD/Difficulty.png");
  service.progress_glow = LoadTexture("resources/gameplay/HUD/ProgressGlow.png");
  SetTextureFilter(service.jacket_img, TEXTURE_FILTER_BILINEAR);

  service.playfield_ratio = app_configs->playfield_ratio;
  text_copy_bounded(service.jacket_path, sizeof(service.jacket_path), "");

  return service;
}

void hud_services_render(HudService *hud_service, RenderContext *render_ctx)
{
  ChartSettings *chart_settings = &render_ctx->chart_settings;
  AudioClock audio_clock = render_ctx->audio_clock;

  float width_ratio         = (float)GetScreenWidth()  / BASE_APP_WINDOW_WIDTH;
  float height_ratio        = (float)GetScreenHeight() / aspect_ratio_get_height(BASE_APP_WINDOW_WIDTH, hud_service->playfield_ratio);
  float hud_dynamic_scaling = Clamp(fminf(width_ratio, height_ratio * 1.8f), 0.7f, 1.4f) * INFO_PANEL_SCALE;
  float info_panel_width    = hud_service->info_panel.width  * hud_dynamic_scaling;
  float info_panel_height   = hud_service->info_panel.height * hud_dynamic_scaling;
  float jacket_bg_width     = hud_service->jacket_bg.width   * hud_dynamic_scaling;
  float info_panel_posX     = (GetScreenWidth() - info_panel_width) / hud_dynamic_scaling;
  float denominator = (float)(audio_clock.total_audio_length) + (chart_settings->audio_offset < 0.0f
                                                                 ? chart_settings->audio_offset : 0.0f);
  float audio_ratio = 0.0f;
  if (fabsf(denominator) > 1e-6)
  {
    audio_ratio = (float)(audio_clock_get_time_ms(&render_ctx->audio_clock) - chart_settings->audio_offset) / denominator;
    audio_ratio = Clamp(audio_ratio, 0.0f, 1.0f);
  }

  // Pause button
  DrawTexture(hud_service->pause_button, -118, 23, WHITE);

  // Info Panel
  rlPushMatrix();
    rlScalef(hud_dynamic_scaling, hud_dynamic_scaling, 1.0f);
    rlTranslatef(0.0f, 16.0f, 0.0f);
    DrawTexture(hud_service->info_panel, info_panel_posX, 0.0f, WHITE);

    // Progress bar + glow
    rlPushMatrix();
      rlTranslatef(info_panel_posX - 70.0f, 0.0f, 0.0f);
      rlPushMatrix();
        rlTranslatef(JACKET_HUD_SIZE + 18.0f, hud_service->info_panel.height * 0.49f, 0.0f);
        float bar_start_x = JACKET_HUD_SIZE + 18.0f;
        float bar_end_x   = 70.0f + hud_service->info_panel.width;
        float progress_glow_x = Lerp(0.0f, bar_end_x - bar_start_x, audio_ratio);
        DrawLineEx(Vector2Zero(), (Vector2){ progress_glow_x, 0.0f }, 5.0f, WHITE);
        DrawTextureEx(hud_service->progress_glow,
                      (Vector2){ progress_glow_x - hud_service->progress_glow.width * 0.5f,
                                -hud_service->progress_glow.height * 0.5f },
                      0.0f, 1.0f, WHITE);
      rlPopMatrix();
    rlPopMatrix();

    // Jacket + Difficulty
    rlPushMatrix();
      rlTranslatef(info_panel_posX - 70.0f, hud_service->info_panel.height * 0.15f, 0.0f);
      DrawTexture(hud_service->jacket_bg, 0.0f, 0.0f, WHITE);
      rlPushMatrix();
        rlTranslatef(18.0f, 18.0f, 0.0f);
        DrawTextureEx(hud_service->jacket_img,
                      Vector2Zero(), 0.0f, JACKET_HUD_SIZE / (float)hud_service->jacket_img.width,
                      WHITE);
        DrawTextureEx(hud_service->jacket_diff,
                      (Vector2){ 0.0f, JACKET_HUD_SIZE }, 0.0f, JACKET_HUD_SIZE / (float)hud_service->jacket_diff.width, 
                      color_from_hex(chart_settings->difficulty_color));


        float diff_spacing = 0.8f;
        float diff_font_size = 39.0f;
        float diff_w = fonts_measure_text(&hud_service->font_with_fallback, chart_settings->difficulty, diff_font_size, diff_spacing);
        float diff_text_scale = diff_w <= (JACKET_HUD_SIZE - 56) ? 1.0f : (JACKET_HUD_SIZE - 56) / diff_w;
        float diff_text_offsetX = (JACKET_HUD_SIZE - (diff_w * diff_text_scale)) / 2;
        BeginShaderMode(hud_service->sdf_shader);
          rlPushMatrix();
            rlTranslatef(diff_text_offsetX, JACKET_HUD_SIZE + 6, 0.0f);
            rlScalef(diff_text_scale, 1.0f, 1.0f);
            fonts_draw_text(&hud_service->font_with_fallback, chart_settings->difficulty, Vector2Zero(), diff_font_size, diff_spacing, WHITE);
          rlPopMatrix();
        EndShaderMode();
      rlPopMatrix();
    rlPopMatrix();

    // Score + Title + Composer
    rlPushMatrix();
      rlTranslatef(info_panel_posX + 180.0f, 0.0f, 0.0f);

      BeginShaderMode(hud_service->sdf_shader);
        rlPushMatrix();
          rlTranslatef(0.0f, 25.0f, 0.0f);
          DrawTextEx(hud_service->saira_medium , "SCORE:", Vector2Zero(), 40.0f, 0, WHITE);
          DrawTextEx(hud_service->saira_regular, "00000000", (Vector2){ -7.0f, 16.0f }, 140.0f, 0, WHITE);
        rlPopMatrix();

        rlPushMatrix();
          rlTranslatef(0.0f, 190.0f, 0.0f);
            float tit_cum_max_width = (info_panel_width - jacket_bg_width + 20) / hud_dynamic_scaling;
          rlPushMatrix();
            float title_w = fonts_measure_text(&hud_service->font_with_fallback, chart_settings->title, 66.0f, 1.0f);
            float title_scale = title_w <= tit_cum_max_width ? 1.0f : tit_cum_max_width / title_w;
            rlScalef(title_scale, 1.0f, 1.0f);
            fonts_draw_text(&hud_service->font_with_fallback, chart_settings->title, Vector2Zero(), 66.0f, 1.0f, WHITE);
          rlPopMatrix();
          rlPushMatrix();
            float composer_w = fonts_measure_text(&hud_service->font_with_fallback, chart_settings->composer, 42.0f, 1.0f);
            float composer_scale = composer_w <= tit_cum_max_width ? 1.0f : tit_cum_max_width / composer_w;
            rlScalef(composer_scale, 1.0f, 1.0f);
            fonts_draw_text(&hud_service->font_with_fallback, chart_settings->composer, (Vector2){ 0.0f, 75.0f }, 42.0f, 1.0f, WHITE);
          rlPopMatrix();
        rlPopMatrix();
      EndShaderMode();
    rlPopMatrix();
  rlPopMatrix();
}

void hud_service_apply_chart(HudService *hud_service, ChartSettings *chart_settings, CachedFontProbes *font_probes)
{
  if (!TextIsEqual(hud_service->jacket_path, chart_settings->jacket_path))
  {
    UnloadTexture(hud_service->jacket_img);
    hud_service->jacket_img = LoadTexture(chart_settings->jacket_path);
    text_copy_bounded(hud_service->jacket_path, sizeof(hud_service->jacket_path), chart_settings->jacket_path);
    if (!IsTextureValid(hud_service->jacket_img))
    {
      hud_service->jacket_img = LoadTexture(DEFAULT_JACKET_PATH);
      text_copy_bounded(hud_service->jacket_path, sizeof(hud_service->jacket_path), DEFAULT_JACKET_PATH);
    }
  }
  SetTextureFilter(hud_service->jacket_img, TEXTURE_FILTER_BILINEAR);

  List missing_codepoints; list_init(&missing_codepoints, sizeof(int));
  font_add_missing_copepoints(&missing_codepoints, chart_settings->title,     &hud_service->font_with_fallback);
  font_add_missing_copepoints(&missing_codepoints, chart_settings->composer,  &hud_service->font_with_fallback);
  font_add_missing_copepoints(&missing_codepoints, chart_settings->difficulty,&hud_service->font_with_fallback);

  if (missing_codepoints.size > 0)
  {
    List fallback_probes; list_init(&fallback_probes, sizeof(FontProbe));
    list_push(&fallback_probes, &font_probes->noto_reg);
    list_push(&fallback_probes, &font_probes->noto_sc_reg);
    list_push(&fallback_probes, &font_probes->noto_jp_reg);
    list_push(&fallback_probes, &font_probes->noto_kr_reg);
    list_push(&fallback_probes, &font_probes->noto_math_reg);

    // If the fallback chain gets too long, the app might lag from drawing
    // --> unload all fonts then reprocess needed codepoints
    if (hud_service->font_with_fallback.size > 16)
    {
      List codepoints; list_init(&codepoints, sizeof(int));
      for (int cp = 0x20; cp <= 0x7E; cp++) list_push(&codepoints, &cp);
      font_add_string_to_codepoints(&codepoints, chart_settings->title);
      font_add_string_to_codepoints(&codepoints, chart_settings->composer);
      font_add_string_to_codepoints(&codepoints, chart_settings->difficulty);
      for (int j = 0; j < hud_service->font_with_fallback.size; j++)
      {
        Font *font = (Font *)list_get(&hud_service->font_with_fallback, j);
        UnloadFont(*font);
      }
      list_free(&hud_service->font_with_fallback);
      hud_service->font_with_fallback = fonts_init(&fallback_probes, 45, (int *)codepoints.data, codepoints.size);

      list_free(&codepoints);
    } else // Only add missing copepoints
    {
      List extra_font = fonts_init(&fallback_probes, 45, (int *)missing_codepoints.data, missing_codepoints.size);
      for (int j = 0; j < extra_font.size; j++)
      {
        Font *font = (Font *)list_get(&extra_font, j);
        list_push(&hud_service->font_with_fallback, font);
      }
      list_free(&extra_font);
    }
    list_free(&fallback_probes);
  }
  list_free(&missing_codepoints);
}

void hud_service_unload(HudService *hud_service)
{
  UnloadFont(hud_service->saira_regular);
  UnloadFont(hud_service->saira_medium);
  UnloadShader(hud_service->sdf_shader);
  UnloadTexture(hud_service->pause_button);
  UnloadTexture(hud_service->info_panel);
  UnloadTexture(hud_service->jacket_bg);
  UnloadTexture(hud_service->jacket_img);
  UnloadTexture(hud_service->jacket_diff);
  UnloadTexture(hud_service->progress_glow);

  for (int i = 0; i < hud_service->font_with_fallback.size; i++)
    UnloadFont(*(Font *)list_get(&hud_service->font_with_fallback, i));
  list_free(&hud_service->font_with_fallback);
}
