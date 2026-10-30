#include <stdlib.h>
#include "raygui.h"
#include "raylib.h"
#include "raymath.h"
#include "resource_util.h"
#include "windows/rgui_input_data.h"
#include "events_panel.h"

static void builtin_sc_option_get_label_text(enum BuiltinSCOption option, char *result, size_t size)
{
  switch (option)
  {
    case EVENTS_PANEL_HIDEGROUP:     text_copy_bounded(result, size, "HIDEGROUP"); break;
    case EVENTS_PANEL_GROUPALPHA:    text_copy_bounded(result, size, "GROUPALPHA"); break;
    case EVENTS_PANEL_ENWIDENCAMERA: text_copy_bounded(result, size, "ENWIDENCAMERA"); break;
    case EVENTS_PANEL_ENWIDENLANES:  text_copy_bounded(result, size, "ENWIDENLANES"); break;
  }
}

EventsPanel events_panel_init()
{
  EventsPanel data = { 0 };
  data.events_option_active = EVENTS_PANEL_TIMINGS;
  data.sc_option = EVENTS_PANEL_HIDEGROUP;
  data.scroll = Vector2Zero();
  data.scroll_rect = (Rectangle){ 0, 0 };
  data.scenecontrol = rgui_selectinput_init("hidegroup;groupalpha;enwidencamera;enwidenlanes", data.sc_option);

  return data;
}

void events_panel_draw(WindowInst* window_inst, EventsPanel *events_panel, Shader sdf_shader, List *timing_groups)
{
  int content_x = window_inst->x + 8;
  int toggle_y  = (int)window_inst->y + 32;
  int scroll_y  = toggle_y + 32;
  int scroll_w  = (int)window_inst->width  - 16;
  int scroll_h  = (int)window_inst->height - (scroll_y - (int)window_inst->y) - 8;

  BeginShaderMode(sdf_shader);

  int groups_panel_h = 150;
  int events_panel_h = 336;

  int input_field_h = 24;
  int input_field_gap = 30;
  int total_content_h = groups_panel_h + events_panel_h + 12*3 + 8;
  GuiScrollPanel((Rectangle){content_x, scroll_y, scroll_w, scroll_h},
                 NULL, (Rectangle){0, 0, scroll_w, total_content_h},
                 &events_panel->scroll, &events_panel->scroll_rect);

  BeginScissorMode(events_panel->scroll_rect.x, events_panel->scroll_rect.y, events_panel->scroll_rect.width + 8, events_panel->scroll_rect.height);

  int scroll_padding = (int)events_panel->scroll_rect.height >= total_content_h ? 16 : 28;

  // Information
  int groups_panel_x = content_x + 8    + (int)events_panel->scroll.x;
  int groups_panel_y = scroll_y  + 12*1 + (int)events_panel->scroll.y;
  int groups_panel_w = scroll_w  - scroll_padding;
  GuiGroupBox ((Rectangle){groups_panel_x, groups_panel_y, groups_panel_w, groups_panel_h}, "[ TIMING GROUPS ]");

  // Events
  int events_panel_x = content_x + 8    + (int)events_panel->scroll.x;
  int events_panel_y = scroll_y  + 12*2 + (int)events_panel->scroll.y + groups_panel_h;
  int events_panel_w = scroll_w  - scroll_padding;
  GuiToggleGroup((Rectangle){events_panel_x, events_panel_y, 56, 24},
                 "Timings;Events", &events_panel->events_option_active);

  switch (events_panel->events_option_active)
  {
    case EVENTS_PANEL_TIMINGS:
    {
      events_panel_y += 24 + 12;
      GuiGroupBox ((Rectangle){events_panel_x, events_panel_y, events_panel_w, events_panel_h - 24}, "[ TIMINGS ]");
      break;
    }
    case EVENTS_PANEL_EVENTS:
    {
      events_panel_y += 24 + 12;
      char result[256]; builtin_sc_option_get_label_text(events_panel->scenecontrol.option, result, 256);
      GuiGroupBox ((Rectangle){events_panel_x, events_panel_y, events_panel_w, events_panel_h - 24}, TextFormat("[ EVENTS - %s ]", result));
      events_panel_y -= 24 + 12;
      rgui_selectinput_dropdown(&events_panel->scenecontrol, (Rectangle){events_panel_x + events_panel_w - 120, events_panel_y, 120, 24});
      events_panel_y += 24 + 12;
      break;
    }
  }

  EndScissorMode();
  EndShaderMode();
}
