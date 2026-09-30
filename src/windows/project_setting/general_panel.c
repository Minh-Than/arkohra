#include "constants.h"
#include "gameplay/camera/camera_service.h"
#include "raygui.h"
#include "raymath.h"
#include "general_panel.h"

GeneralSettingPanel general_setting_panel_init(AppConfigs *app_configs)
{
  GeneralSettingPanel data = { 0 };
  data.scroll = Vector2Zero();
  data.scroll_rect = (Rectangle){ 0, 0 };

  snprintf(data.scroll_speed.text, sizeof(data.scroll_speed.text), "%.*f", PROJ_FLOAT_DECIMALS, app_configs->scroll_speed);
  data.scroll_speed = rgui_floatinput_init(data.scroll_speed.text);
  data.aspect_ratio = rgui_selectinput_init("16 : 9;20 : 9;18 : 9;4 : 3;3 : 2", app_configs->playfield_ratio);
  data.colorblind = app_configs->colorblind;

  snprintf(data.music_volume.text, sizeof(data.music_volume.text), "%.*f", PROJ_FLOAT_DECIMALS, app_configs->music_volume);
  data.music_volume = rgui_floatinput_init(data.music_volume.text);
  snprintf(data.effect_volume.text, sizeof(data.effect_volume.text), "%.*f", PROJ_FLOAT_DECIMALS, app_configs->hit_volume);
  data.effect_volume = rgui_floatinput_init(data.effect_volume.text);

  return data;
}

void general_setting_panel_draw(WindowInst* window_inst, GeneralSettingPanel *gen_set_panel,
                                AppConfigs *app_configs, RenderContext *render_ctx,
                                Shader sdf_shader, List *font_fallbacks)
{
  int content_x = window_inst->x + 8;
  int toggle_y  = (int)window_inst->y + 32;
  int scroll_y  = toggle_y + 32;
  int scroll_w  = (int)window_inst->width  - 16;
  int scroll_h  = (int)window_inst->height - (scroll_y - (int)window_inst->y) - 8;

  BeginShaderMode(sdf_shader);

  int gameplay_panel_h = 110;
  int audio_panel_h = 80;

  int input_field_h = 24;
  int input_field_gap = 30;
  int total_content_h = gameplay_panel_h +audio_panel_h + 12*2 + 8;
  GuiScrollPanel((Rectangle){content_x, scroll_y, scroll_w, scroll_h},
                 NULL, (Rectangle){0, 0, scroll_w, total_content_h},
                 &gen_set_panel->scroll, &gen_set_panel->scroll_rect);

  int label_xs = 96;
  BeginScissorMode(gen_set_panel->scroll_rect.x, gen_set_panel->scroll_rect.y, gen_set_panel->scroll_rect.width, gen_set_panel->scroll_rect.height);

  // Audio
  int audio_panel_x = content_x + 8    + (int)gen_set_panel->scroll.x;
  int audio_panel_y = scroll_y  + 12*2 + (int)gen_set_panel->scroll.y + gameplay_panel_h;
  int audio_panel_w = scroll_w  - 28;
  GuiGroupBox ((Rectangle){audio_panel_x, audio_panel_y, audio_panel_w, audio_panel_h}, "[ AUDIO ]");

  int audio_field_x = audio_panel_x + 8;
  int audio_field_y = audio_panel_y + 16;
  int audio_input_x = audio_field_x + label_xs;

  Rectangle audio_label_rect = { audio_field_x, audio_field_y, label_xs, input_field_h };
  Rectangle audio_field_rect = { audio_input_x, audio_field_y, audio_panel_w - 16 - label_xs, input_field_h };

  GuiLabel(audio_label_rect, "Music Volume");
  rgui_floatinput_textbox(&gen_set_panel->music_volume, audio_field_rect, PROJ_FLOAT_DECIMALS);

  audio_label_rect.y += input_field_gap;
  audio_field_rect.y += input_field_gap;
  GuiLabel(audio_label_rect, "Effect Volume");
  rgui_floatinput_textbox(&gen_set_panel->effect_volume, audio_field_rect, PROJ_FLOAT_DECIMALS);

  // Gameplay
  // WARNING: the dropdown area are treated in rendering the same way, meaning anything drawn after WILL BE DRAWN ON TOP OF IT
  int gameplay_panel_x = content_x + 8    + (int)gen_set_panel->scroll.x;
  int gameplay_panel_y = scroll_y  + 12*1 + (int)gen_set_panel->scroll.y;
  int gameplay_panel_w = scroll_w  - 28;
  GuiGroupBox ((Rectangle){gameplay_panel_x, gameplay_panel_y, gameplay_panel_w, gameplay_panel_h}, "[ GAMEPLAY ]");

  int gameplay_field_x = gameplay_panel_x + 8;
  int gameplay_field_y = gameplay_panel_y + 16;
  int gameplay_input_x = gameplay_field_x + label_xs;

  Rectangle gameplay_label_rect = { gameplay_field_x, gameplay_field_y, label_xs, input_field_h };
  Rectangle gameplay_field_rect = { gameplay_input_x, gameplay_field_y, gameplay_panel_w - 16 - label_xs, input_field_h };

  GuiLabel(gameplay_label_rect, "Chart Speed");
  rgui_floatinput_textbox(&gen_set_panel->scroll_speed, gameplay_field_rect, PROJ_FLOAT_DECIMALS);

  gameplay_label_rect.y += input_field_gap*2;
  gameplay_field_rect.y += input_field_gap*2;
  if(GuiCheckBox((Rectangle){ gameplay_field_x, gameplay_label_rect.y, input_field_h, input_field_h},
              "Use Colorblind Arc Colors", &gen_set_panel->colorblind))
  {
    app_configs->colorblind = gen_set_panel->colorblind;
  }

  gameplay_label_rect.y -= input_field_gap;
  gameplay_field_rect.y -= input_field_gap;
  GuiLabel(gameplay_label_rect, "Aspect Ratio");
  EndScissorMode();
  if (rgui_selectinput_dropdown(&gen_set_panel->aspect_ratio, gameplay_field_rect))
  {
    app_configs->playfield_ratio = gen_set_panel->aspect_ratio.option;
    SetWindowSize(GetScreenWidth(), (int)aspect_ratio_get_height((float)GetScreenWidth(), app_configs->playfield_ratio));
    recalibrate_camera(&render_ctx->camera);
  }
  EndShaderMode();

}
