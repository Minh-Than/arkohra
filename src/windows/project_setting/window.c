#include <math.h>
#include <string.h>
#include "raygui.h"
#include "raylib.h"
#include "data/fonts/fonts_service.h"
#include "../project_setting/window.h"

ProjSettingData proj_setting_init(int glsl, AppConfigs *app_configs)
{
  ProjSettingData data = { 0 };
  data.setting_options_active = 2;

  data.project_panel = project_panel_init();
  data.gen_set_panel = general_setting_panel_init(app_configs);

  data.sdf_shader = LoadShader(0, TextFormat("resources/shaders/glsl%i/sdf.fs", glsl));
  const char *paths[] = { "resources/fonts/NotoSans-Regular.ttf", };
  List font_list; list_init(&font_list, sizeof(char *));
  for (int i = 0; i < 1; i++) list_push(&font_list, &paths[i]);
  List hud_code_points; list_init(&hud_code_points, sizeof(int));
  for (int cp = 0x20; cp <= 0x7E; cp++) list_push(&hud_code_points, &cp);
  data.font_fallbacks = fonts_init(&font_list, 45, (int *)hud_code_points.data, hud_code_points.size); 
  list_free(&hud_code_points);
  list_free(&font_list);

  return data;
}

void proj_setting_draw(WindowInst* window_inst, ProjSettingData *data, AppConfigs *app_configs, RenderContext *render_ctx)
{
  if (!window_inst->is_visible) return;
  Rectangle win_rect = (Rectangle){ window_inst->x, window_inst->y,
                                    window_inst->width, window_inst->height };

  static bool resizing = false;
  Rectangle handle  = (Rectangle){ win_rect.x + win_rect.width  - 10,
                                   win_rect.y + win_rect.height - 10,
                                   20, 20 };
  if (resizing)
  {
    window_inst->width  = fmaxf(400, fminf(GetScreenWidth() , GetMouseX() - window_inst->x));
    window_inst->height = fmaxf(400, fminf(GetScreenHeight(), GetMouseY() - window_inst->y));
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) resizing = false;
  }
  else if (CheckCollisionPointRec(GetMousePosition(), handle) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    resizing = true;

  static bool dragging = false;
  static Vector2 dragging_pos = { 0, 0 };
  Rectangle top_bar = (Rectangle){ win_rect.x, win_rect.y, win_rect.width, 24 };
  if(dragging)
  {
    window_inst->x = GetMouseX() - dragging_pos.x;
    window_inst->y = GetMouseY() - dragging_pos.y;
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) dragging = false;
  }
  else if (CheckCollisionPointRec(GetMousePosition(), top_bar) &&
           IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
  {
    dragging = true;
    dragging_pos.x = GetMouseX() - window_inst->x;
    dragging_pos.y = GetMouseY() - window_inst->y;
  }

  DrawTriangle((Vector2){ handle.x + 8, handle.y + 8 },
               (Vector2){ handle.x + 8, handle.y - 1 },
               (Vector2){ handle.x - 2, handle.y + 8 }, GRAY);

  data->setting_window_active = !GuiWindowBox(win_rect, "PROJECT SETTINGS");
  if (!data->setting_window_active) window_inst_close(window_inst);

  // --- layout, relative to the window ---
  int content_x = window_inst->x + 8;
  int toggle_y  = (int)window_inst->y + 32;

  GuiToggleGroup((Rectangle){content_x, toggle_y, 56, 24},
                 "Project;Events;Settings", &data->setting_options_active);

  switch (data->setting_options_active)
  {
    case SETTING_PROJECT:
      project_panel_draw(window_inst, &data->project_panel, app_configs, render_ctx, data->sdf_shader, &data->font_fallbacks);
      break;
    case SETTING_EVENTS:
      GuiLabel((Rectangle){content_x, toggle_y + 32, 50, 10}, "Balls");
      break;
    case SETTING_GENERAL:
      general_setting_panel_draw(window_inst, &data->gen_set_panel, app_configs, render_ctx, data->sdf_shader, &data->font_fallbacks);
      break;
  }

  if (data->project_panel.is_text_edited) proj_setting_reload_font(data);
}

void proj_setting_unload(ProjSettingData *proj_setting)
{
  project_panel_unload(&proj_setting->project_panel);

  for (int i = 0; i < proj_setting->font_fallbacks.size; i++)
  {
    Font *font = (Font *)list_get(&proj_setting->font_fallbacks, i);
    UnloadFont(*font);
  }
  list_free(&proj_setting->font_fallbacks);
}

void proj_setting_reload_font(ProjSettingData *proj_setting)
{
  List missing_codepoints; list_init(&missing_codepoints, sizeof(int));
  project_panel_add_missing_codepoints(&proj_setting->project_panel, &missing_codepoints, &proj_setting->font_fallbacks);

  if (missing_codepoints.size > 0)
  {
    const char *paths[] = {
      "resources/fonts/NotoSans-Regular.ttf",
      "resources/fonts/NotoSansSC-Regular.ttf",
      "resources/fonts/NotoSansJP-Regular.ttf",
      "resources/fonts/NotoSansKR-Regular.ttf",
      "resources/fonts/NotoSansMath-Regular.ttf",
    };
    List font_list; list_init(&font_list, sizeof(char *));
    for (int j = 0; j < 5; j++) list_push(&font_list, &paths[j]);

    // If the fallback chain gets too long, the app might lag from drawing
    // --> unload all fonts then reprocess needed codepoints
    if (proj_setting->font_fallbacks.size > 16)
    {
      List hud_code_points; list_init(&hud_code_points, sizeof(int));
      project_panel_add_str_to_codepoints(&proj_setting->project_panel, &hud_code_points);
      for (int cp = 0x20; cp <= 0x7E; cp++) list_push(&hud_code_points, &cp);
      for (int j = 0; j < proj_setting->font_fallbacks.size; j++)
      {
        Font *font = (Font *)list_get(&proj_setting->font_fallbacks, j);
        UnloadFont(*font);
      }
      list_free(&proj_setting->font_fallbacks);
      proj_setting->font_fallbacks = fonts_init(&font_list, 45, (int *)hud_code_points.data, hud_code_points.size);

      list_free(&hud_code_points);
    } else // Only add missing copepoints
    {
      List extra_font = fonts_init(&font_list, 45, (int *)missing_codepoints.data, missing_codepoints.size);
      for (int j = 0; j < extra_font.size; j++)
      {
        Font *font = (Font *)list_get(&extra_font, j);
        list_push(&proj_setting->font_fallbacks, font);
      }
      list_free(&extra_font);
    }
    list_free(&font_list);
  }
  list_free(&missing_codepoints);

  if (proj_setting->project_panel.is_text_edited) proj_setting->project_panel.is_text_edited = false;
}

void proj_setting_apply_chart(ProjSettingData *project_setting, ChartSettings *chart_settings)
{
  for (int i = 0; i < project_setting->font_fallbacks.size; i++)
  {
    Font *font = (Font *)list_get(&project_setting->font_fallbacks, i);
    UnloadFont(*font);
  }
  list_free(&project_setting->font_fallbacks);

  project_panel_apply_chart(&project_setting->project_panel, chart_settings);
  proj_setting_reload_font(project_setting);
}
