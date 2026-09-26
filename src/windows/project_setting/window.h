#ifndef WIN_PROJECT_SETTING
#define WIN_PROJECT_SETTING

#include "raylib.h"
#include "data/app_configs/app_config.h"
#include "render/render_service.h"
#include "windows/rgui_input_data.h"
#include "windows/window_inst.h"

enum ProjTabOption {
  SETTING_PROJECT = 0,
  SETTING_EVENTS  = 1,
  SETTING_GENERAL = 2,
};

typedef struct
{
  bool  setting_window_active;
  int   setting_options_active;

  // PROJECT
  // Info
  RguiTextInput title;
  RguiTextInput composer;
  RguiTextInput illustrator;
  RguiTextInput charter;
  RguiTextInput diff_text;
  RguiTextInput alias;

  // Gameplay
  RguiFloatInput base_bpm;
  bool is_sync;
  RguiTextInput bpm_text;
  RguiIntInput chart_offset;
  RguiIntInput judge_density;
  RguiFloatInput cc;
  RguiIntInput preview_from, preview_to;
  RguiTextInput search_tag;

  // Files
  RguiFileInput audio;
  RguiFileInput jacket;
  RguiFileInput background;
  RguiFileInput bg_video;

  Rectangle scroll_rect;
  Vector2 proj_scroll, general_scroll;

  // GENERAL
  // Gameplay
  RguiFloatInput scroll_speed;
  RguiSelectInput aspect_ratio;
  bool colorblind;

  // Audio
  RguiFloatInput music_volume;
  RguiFloatInput effect_volume;

  Shader sdf_shader;
  List font_fallbacks;
} ProjSettingData;

ProjSettingData proj_setting_init(int glsl, AppConfigs *app_configs);
void proj_setting_draw(WindowInst* window_inst, ProjSettingData *data, AppConfigs *app_configs, RenderContext *render_ctx);
void proj_setting_unload(ProjSettingData *proj_setting);
void proj_setting_reload_font(ProjSettingData *proj_setting);

#endif // WIN_PROJECT_SETTING
