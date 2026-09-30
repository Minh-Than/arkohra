#ifndef PROJECT_PANEL_H
#define PROJECT_PANEL_H

#include <stdbool.h>
#include "raylib.h"
#include "data/app_configs/app_config.h"
#include "data/custom_types/dynamic_list.h"
#include "render/render_service.h"
#include "windows/rgui_input_data.h"
#include "windows/window_inst.h"

typedef struct {
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
  RguiTextInput search_tags;

  // Files
  RguiFileInput audio;
  RguiFileInput jacket;
  RguiFileInput background;
  RguiFileInput bg_video;

  Rectangle scroll_rect;
  Vector2 scroll;

  bool is_text_edited;
} ProjectPanel;

ProjectPanel project_panel_init();
void project_panel_draw(WindowInst* window_inst, ProjectPanel *project_panel,
                        AppConfigs *app_configs, RenderContext *render_ctx,
                        Shader sdf_shader, List *font_fallbacks);
void project_panel_add_missing_codepoints(ProjectPanel *project_panel, List *codepoints, List *font_fallbacks);
void project_panel_add_str_to_codepoints(ProjectPanel *project_panel, List *codepoints);
void project_panel_apply_chart(ProjectPanel *project_panel, ChartSettings *chart_settings);
void project_panel_unload(ProjectPanel *project_panel);

#endif // PROJECT_PANEL_H
