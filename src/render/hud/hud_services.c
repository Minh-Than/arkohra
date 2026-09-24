#include <math.h>
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

HudService hud_service_init(int glsl)
{
  HudService service = { 0 };
  service.sdf_shader    = LoadShader(0, TextFormat("resources/shaders/glsl%i/sdf.fs", glsl));
  service.saira_regular = font_generate_sdf((char *)"resources/fonts/Saira-Regular.ttf", 45, NULL, 95);
  service.saira_medium  = font_generate_sdf((char *)"resources/fonts/Saira-Medium.ttf", 45, NULL, 95);

  const char *paths[] = { "resources/fonts/NotoSans-Regular.ttf" };
  List font_list; list_init(&font_list, sizeof(char *));
  for (int i = 0; i < 1; i++) list_push(&font_list, &paths[i]);

  List hud_code_points; list_init(&hud_code_points, sizeof(int));
  for (int cp = 0x20; cp <= 0x7E; cp++) list_push(&hud_code_points, &cp);
  service.font_with_fallback = fonts_init(&font_list, 45, (int *)hud_code_points.data, hud_code_points.size);
  list_free(&hud_code_points);
  list_free(&font_list);

  service.pause_button  = LoadTexture("resources/gameplay/HUD/PauseLight.png");
  service.info_panel    = LoadTexture("resources/gameplay/HUD/InfoLight.png");
  service.jacket_bg     = LoadTexture("resources/gameplay/HUD/JacketBackground.png");
  service.jacket_img    = LoadTexture("resources/gameplay/DefaultJacket.png");
  service.jacket_diff   = LoadTexture("resources/gameplay/HUD/Difficulty.png");
  service.progress_glow = LoadTexture("resources/gameplay/HUD/ProgressGlow.png");
  SetTextureFilter(service.jacket_img, TEXTURE_FILTER_BILINEAR);

  return service;
}

void hud_services_render(HudService *hud_service, ChartSettings *chart_settings)
{
  // TODO: move these shits away from calculating each frame
  float width_ratio         = (float)GetScreenWidth()  / 1280;
  float height_ratio        = (float)GetScreenHeight() / 720;
  float hud_dynamic_scaling = Clamp(fminf(width_ratio, height_ratio * 1.8f), 0.8f, 1.4f) * INFO_PANEL_SCALE;
  float info_panel_width    = hud_service->info_panel.width  * hud_dynamic_scaling;
  float info_panel_height   = hud_service->info_panel.height * hud_dynamic_scaling;
  float jacket_bg_width     = hud_service->jacket_bg.width   * hud_dynamic_scaling;
  float info_panel_posX     = (GetScreenWidth() - info_panel_width) / hud_dynamic_scaling;

  // Pause button
  DrawTexture(hud_service->pause_button, -118, 23, WHITE);

  // Info Panel
  rlPushMatrix();
    rlScalef(hud_dynamic_scaling, hud_dynamic_scaling, 1.0f);
    rlTranslatef(0.0f, 16.0f, 0.0f);
    DrawTexture(hud_service->info_panel, info_panel_posX, 0.0f, WHITE);

    // TODO: fuck this one in particular
    // Progress bar + glow
    // rlPushMatrix();
    //   rlTranslatef(info_panel_posX - 70.0f, 0.0f, 0.0f);
    //   rlPushMatrix();
    //     rlTranslatef(JACKET_HUD_SIZE + 18.0f, hud_service->info_panel.height * 0.49f, 0.0f);
    //     float progress_glow_x = (hud_service->info_panel.width - JACKET_HUD_SIZE) * (current_ms / audio_clock->total_audio_length);
    //     DrawLineEx(Vector2Zero(), (Vector2){ progress_glow_x, 0.0f }, 5.0f, WHITE);
    //     DrawTextureEx(hud_service->progress_glow,
    //                   (Vector2){ progress_glow_x - hud_service->progress_glow.width * 0.5f,
    //                             -hud_service->progress_glow.height * 0.5f },
    //                   0.0f, 1.0f, WHITE);
    //   rlPopMatrix();
    // rlPopMatrix();

    rlPushMatrix();
      rlTranslatef(info_panel_posX - 70.0f, info_panel_height * 0.25f, 0.0f);


      // Jacket + Difficulty
      DrawTexture(hud_service->jacket_bg, 0.0f, 0.0f, WHITE);
      rlPushMatrix();
        rlTranslatef(18.0f, 18.0f, 0.0f);
        DrawTextureEx(hud_service->jacket_img,
                      Vector2Zero(), 0.0f, JACKET_HUD_SIZE / (float)hud_service->jacket_img.width,
                      WHITE);
        DrawTextureEx(hud_service->jacket_diff,
                      (Vector2){ 0.0f, JACKET_HUD_SIZE }, 0.0f, JACKET_HUD_SIZE / (float)hud_service->jacket_diff.width, 
                      color_from_hex(chart_settings->difficulty_color));


        float diff_spacing = 1.0f;
        float diff_font_size = 44.0f;
        float diff_w = fonts_measure_text(&hud_service->font_with_fallback, chart_settings->difficulty, diff_font_size, diff_spacing);
        float diff_text_scale = diff_w <= (JACKET_HUD_SIZE - 56) ? 1.0f : (JACKET_HUD_SIZE - 56) / diff_w;
        float diff_text_offsetX = (JACKET_HUD_SIZE - (diff_w * diff_text_scale)) / 2;
        BeginShaderMode(hud_service->sdf_shader);
          rlPushMatrix();
            rlTranslatef(diff_text_offsetX, JACKET_HUD_SIZE + 3, 0.0f);
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
