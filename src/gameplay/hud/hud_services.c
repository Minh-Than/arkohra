#include <math.h>
#include "color_services.h"
#include "constants.h"
#include "hud_services.h"
#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

void hud_services_render(TextureGroup *texture_group, ChartSettings *chart_settings, FontServices *font_services)
{
  // TODO: move these shits away from calculating each frame
  float width_ratio         = (float)GetScreenWidth()  / 1280;
  float height_ratio        = (float)GetScreenHeight() / 720;
  float hud_dynamic_scaling = Clamp(fminf(width_ratio, height_ratio * 1.8f), 0.8f, 1.4f) * INFO_PANEL_SCALE;
  float info_panel_width    = texture_group->info_panel.width  * hud_dynamic_scaling;
  float info_panel_height   = texture_group->info_panel.height * hud_dynamic_scaling;
  float jacket_bg_width     = texture_group->jacket_bg.width   * hud_dynamic_scaling;
  float jacket_bg_height    = texture_group->jacket_bg.height  * hud_dynamic_scaling;
  float info_panel_posX     = (GetScreenWidth() - info_panel_width) / hud_dynamic_scaling;

  // Pause button
  DrawTexture(texture_group->pause_button, -118, 23, WHITE);

  // Info Panel
  rlPushMatrix();
    rlScalef(hud_dynamic_scaling, hud_dynamic_scaling, 1.0f);
    rlTranslatef(0.0f, 16.0f, 0.0f);
    DrawTexture(texture_group->info_panel, info_panel_posX, 0.0f, WHITE);

    // Jacket + Difficulty
    rlPushMatrix();
      rlTranslatef(info_panel_posX - 70.0f, info_panel_height * 0.25f, 0.0f);

      DrawTexture(texture_group->jacket_bg, 0.0f, 0.0f, WHITE);
      rlPushMatrix();
        rlTranslatef(18.0f, 18.0f, 0.0f);
        DrawTextureEx(texture_group->jacket_img,
                      Vector2Zero(), 0.0f, JACKET_HUD_SIZE / (float)texture_group->jacket_img.width,
                      WHITE);
        DrawTextureEx(texture_group->jacket_diff,
                      (Vector2){ 0.0f, JACKET_HUD_SIZE }, 0.0f, JACKET_HUD_SIZE / (float)texture_group->jacket_diff.width, 
                      color_from_hex(chart_settings->difficulty_color));

        float diff_spacing = 1.0f;
        float diff_text_width = JACKET_HUD_SIZE;
        Vector2 diff_v = MeasureTextEx(font_services->hud_notosans_tc_reg, chart_settings->difficulty, 44.0f, diff_spacing);
        float diff_text_scale = diff_v.x <= (diff_text_width - 50) ? 1.0f : (diff_text_width - 50) / diff_v.x;
        float diff_text_offsetX = (diff_text_width - (diff_v.x * diff_text_scale)) / 2;
        BeginShaderMode(font_services->hud_sdf_shader);
          rlPushMatrix();
            //Vector2 diff_v = MeasureTextEx(font_services->hud_notosans_tc_reg, chart_settings->difficulty, 46.0f, 0);
            rlTranslatef(diff_text_offsetX, 0.0f, 0.0f);
            rlScalef(diff_text_scale, 1.0f, 1.0f);
            DrawTextEx(font_services->hud_notosans_tc_reg, chart_settings->difficulty, (Vector2) { 0.0f, JACKET_HUD_SIZE }, 44.0f, diff_spacing, WHITE);
          rlPopMatrix();
        EndShaderMode();
      rlPopMatrix();
    rlPopMatrix();

    // Score + Title + Composer
    rlPushMatrix();
      rlTranslatef(info_panel_posX + 180.0f, 0.0f, 0.0f);

      BeginShaderMode(font_services->hud_sdf_shader);
        rlPushMatrix();
          rlTranslatef(0.0f, 25.0f, 0.0f);
          DrawTextEx(font_services->saira_medium , "SCORE:", Vector2Zero(), 40.0f, 0, WHITE);
          DrawTextEx(font_services->saira_regular, "00000000", (Vector2){ -7.0f, 16.0f }, 140.0f, 0, WHITE);
        rlPopMatrix();

        rlPushMatrix();
          rlTranslatef(0.0f, 190.0f, 0.0f);
            float tit_cum_max_width = (info_panel_width - jacket_bg_width + 20) / hud_dynamic_scaling;
          rlPushMatrix();
            Vector2 title_v = MeasureTextEx(font_services->hud_notosans_tc_reg, chart_settings->title, 66.0f, 1.0f);
            float title_scale = title_v.x <= tit_cum_max_width ? 1.0f : tit_cum_max_width/title_v.x;
            rlScalef(title_scale, 1.0f, 1.0f);
            DrawTextEx(font_services->hud_notosans_tc_reg, chart_settings->title, Vector2Zero(), 66.0f, 1.0f, WHITE);
          rlPopMatrix();
          rlPushMatrix();
            Vector2 composer_v = MeasureTextEx(font_services->hud_notosans_tc_reg, chart_settings->composer, 42.0f, 1.0f);
            float composer_scale = composer_v.x <= tit_cum_max_width ? 1.0f : tit_cum_max_width/composer_v.x;
            rlScalef(composer_scale, 1.0f, 1.0f);
            DrawTextEx(font_services->hud_notosans_tc_reg, chart_settings->composer, (Vector2){ 0.0f, 75.0f }, 42.0f, 1.0f, WHITE);
          rlPopMatrix();
        rlPopMatrix();
      EndShaderMode();
    rlPopMatrix();
  rlPopMatrix();
}
