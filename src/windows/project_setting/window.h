#ifndef WIN_PROJECT_SETTING
#define WIN_PROJECT_SETTING

#include "raylib.h"
#include "data/app_configs/app_config.h"
#include "data/chart_settings/chart_settings.h"
#include "render/render_service.h"
#include "windows/project_setting/general_panel.h"
#include "windows/project_setting/project_panel.h"

enum ProjTabOption {
  SETTING_PROJECT = 0,
  SETTING_EVENTS  = 1,
  SETTING_GENERAL = 2,
};

typedef struct
{
  bool setting_window_active;
  int  setting_options_active;

  ProjectPanel project_panel;
  GeneralSettingPanel gen_set_panel;

  Shader sdf_shader;
  List font_fallbacks;
} ProjSettingData;

ProjSettingData proj_setting_init(int glsl, AppConfigs *app_configs);
void proj_setting_draw(WindowInst* window_inst, ProjSettingData *data, AppConfigs *app_configs, RenderContext *render_ctx);
void proj_setting_unload(ProjSettingData *proj_setting);
void proj_setting_reload_font(ProjSettingData *proj_setting);
void proj_setting_apply_chart(ProjSettingData *project_setting, ChartSettings *chart_settings);

#endif // WIN_PROJECT_SETTING
