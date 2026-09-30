#ifndef GENERAL_SETTING_PANEL_H
#define GENERAL_SETTING_PANEL_H

#include "data/app_configs/app_config.h"
#include "data/custom_types/dynamic_list.h"
#include "raylib.h"
#include "render/render_service.h"
#include "windows/rgui_input_data.h"
#include "windows/window_inst.h"

typedef struct {
  // Gameplay
  RguiFloatInput scroll_speed;
  RguiSelectInput aspect_ratio;
  bool colorblind;

  // Audio
  RguiFloatInput music_volume;
  RguiFloatInput effect_volume;

  Rectangle scroll_rect;
  Vector2 scroll;
} GeneralSettingPanel;

GeneralSettingPanel general_setting_panel_init(AppConfigs *app_configs);
void general_setting_panel_draw(WindowInst* window_inst, GeneralSettingPanel *gen_set_panel,
                                AppConfigs *app_configs, RenderContext *render_ctx,
                                Shader sdf_shader, List *font_fallbacks);

#endif // GENERAL_SETTING_PANEL_H
