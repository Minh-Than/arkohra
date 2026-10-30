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

  RguiSelectInput scenecontrol;

  Rectangle scroll_rect;
  Vector2 scroll;
} EventsPanel;

EventsPanel events_panel_init();
void events_panel_draw(WindowInst* window_inst, EventsPanel *events_panel, Shader sdf_shader, List *timing_groups);

#endif // EVENTS_PANEL_H
