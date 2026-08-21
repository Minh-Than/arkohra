#include <math.h>
#include "color_services.h"
#include "constants.h"
#include "hud_services.h"
#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

static void draw_fit_text(Font font, const char *text, Vector2 pos, float font_size, float max_width, Color color)
{
  Vector2 m = MeasureTextEx(font, text, font_size, 0);
  float scale = m.x <= max_width ? 1.0f : max_width / m.x;
  rlPushMatrix();
    rlTranslatef(pos.x, pos.y, 0.0f);
    rlScalef(scale, 1.0f, 1.0f);
    DrawTextEx(font, text, Vector2Zero(), font_size, 0, color);
  rlPopMatrix();
}

void hud_services_render(TextureGroup *texture_group, ChartSettings *chart_settings, FontServices *font_services)
{
  // TODO: move these calculations out of the per-frame path.
  float scale_w  = (float)GetScreenWidth()  / 1280;
  float scale_h  = (float)GetScreenHeight() / 720;
  float hud_scale = Clamp(fminf(scale_w, scale_h * 1.8f), 0.8f, 1.4f) * INFO_PANEL_SCALE;

  float panel_w  = texture_group->info_panel.width  * hud_scale;
  float panel_h  = texture_group->info_panel.height * hud_scale;
  float jacket_w = texture_group->jacket_bg.width   * hud_scale;
  float panel_x  = (GetScreenWidth() - panel_w) / hud_scale;

  // Pause button (unscaled)
  DrawTexture(texture_group->pause_button, -118, 23, WHITE);

  rlPushMatrix();
    rlScalef(hud_scale, hud_scale, 1.0f);
    rlTranslatef(0.0f, 16.0f, 0.0f);
    DrawTexture(texture_group->info_panel, panel_x, 0.0f, WHITE);

    // Jacket + Difficulty
    rlPushMatrix();
      rlTranslatef(panel_x - 70.0f, panel_h * 0.25f, 0.0f);
      DrawTexture(texture_group->jacket_bg, 0.0f, 0.0f, WHITE);

      rlPushMatrix();
        rlTranslatef(18.0f, 18.0f, 0.0f);

        float img_scale  = JACKET_HUD_SIZE / (float)texture_group->jacket_img.width;
        float diff_scale = JACKET_HUD_SIZE / (float)texture_group->jacket_diff.width;

        // Jacket
        DrawTextureEx(texture_group->jacket_img, Vector2Zero(), 0.0f, img_scale, WHITE);
        DrawTextureEx(texture_group->jacket_diff, (Vector2){ 0, JACKET_HUD_SIZE }, 0.0f, diff_scale, color_from_hex(chart_settings->difficulty_color));

        // Difficulty text
        Vector2 diff_m = MeasureTextEx(font_services->hud_notosans_tc_reg, chart_settings->difficulty, 44.0f, 0);
        float diff_max = JACKET_HUD_SIZE - 50.0f;
        float diff_off = (JACKET_HUD_SIZE - diff_m.x) * 0.5f;

        BeginShaderMode(font_services->hud_sdf_shader);
        if (diff_m.x > diff_max)
        {
          float s = diff_max / diff_m.x;
          diff_off = (JACKET_HUD_SIZE - diff_m.x * s) * 0.5f;
          rlPushMatrix();
            rlTranslatef(diff_off, JACKET_HUD_SIZE, 0.0f);
            rlScalef(s, 1.0f, 1.0f);
              DrawTextEx(font_services->hud_notosans_tc_reg, chart_settings->difficulty, Vector2Zero(), 44.0f, 0, WHITE);
          rlPopMatrix();
        } else 
        {
          DrawTextEx(font_services->hud_notosans_tc_reg, chart_settings->difficulty, (Vector2){ diff_off, JACKET_HUD_SIZE }, 44.0f, 0, WHITE);
        }
        EndShaderMode();
      rlPopMatrix();
    rlPopMatrix();

    // Score + Title + Composer
    rlPushMatrix();
      rlTranslatef(panel_x + 180.0f, 0.0f, 0.0f);

      float max_w = (panel_w - jacket_w + 20.0f) / hud_scale;

      BeginShaderMode(font_services->hud_sdf_shader);
        // Score
        DrawTextEx(font_services->saira_medium , "SCORE:"  , (Vector2){ 0, 25 }, 40.0f, 0, WHITE);
        DrawTextEx(font_services->saira_regular, "00000000", (Vector2){ -7, 41 }, 140.0f, 0, WHITE);

        // Title + Composer
        rlPushMatrix();
          rlTranslatef(0.0f, 190.0f, 0.0f);
          draw_fit_text(font_services->hud_notosans_tc_reg, chart_settings->title   , Vector2Zero(), 66.0f, max_w, WHITE);
          draw_fit_text(font_services->hud_notosans_tc_reg, chart_settings->composer, (Vector2){ 0, 75 }, 42.0f, max_w, WHITE);
        rlPopMatrix();
      EndShaderMode();
    rlPopMatrix();
  rlPopMatrix();
}
