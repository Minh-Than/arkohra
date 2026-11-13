#ifndef EVENTS_PANEL_H
#define EVENTS_PANEL_H

#include "raylib.h"
#include "data/custom_types/dynamic_list.h"
#include "windows/rgui_input_data.h"
#include "windows/window_inst.h"

enum EventsTabOption {
  EVENTS_PANEL_TIMINGS = 0,
  EVENTS_PANEL_EVENTS  = 1,
};

enum BuiltinSCOption {
  EVENTS_PANEL_HIDEGROUP     = 0,
  EVENTS_PANEL_GROUPALPHA    = 1,
  EVENTS_PANEL_ENWIDENCAMERA = 2,
  EVENTS_PANEL_ENWIDENLANES  = 3,
};

typedef struct {
  int events_option_active;
  enum BuiltinSCOption sc_option;
  int curr_tg;

  RguiSelectInput scenecontrol;

  Rectangle scroll_rect, tg_scroll_rect, event_scroll_rect;
  Vector2 scroll, tg_scroll, event_scroll;

} EventsPanel;

EventsPanel events_panel_init();
void events_panel_draw(WindowInst* window_inst, EventsPanel *events_panel, bool is_chart_initiailzed, Shader sdf_shader, List *timing_groups);
void events_panel_apply_chart(EventsPanel *events_panel);

#endif // EVENTS_PANEL_H
