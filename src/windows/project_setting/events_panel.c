#include <stdlib.h>
#include <stdio.h>
#include "data/chart_timing_groups/chart_timing_group.h"
#include "data/gameplay_events/timing_event.h"
#include "data/keyframe/value_channel.h"
#include "raygui.h"
#include "raylib.h"
#include "raymath.h"
#include "resource_util.h"
#include "windows/rgui_input_data.h"
#include "windows/window_inst.h"
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
  data.scroll_rect = data.tg_scroll_rect = data.event_scroll_rect = (Rectangle){ 0, 0 };
  data.scroll = data.tg_scroll = data.event_scroll = Vector2Zero();
  data.scenecontrol = rgui_selectinput_init("hidegroup;groupalpha;enwidencamera;enwidenlanes", EVENTS_PANEL_HIDEGROUP);
  data.timings_events = rgui_selectinput_init("Timings;Events", EVENTS_PANEL_TIMINGS);

  return data;
}

static void draw_timing_groups_panel_list(List *timing_groups, WindowInst *window_inst, EventsPanel *events_panel,
                                         int groups_panel_x, int groups_panel_y, int groups_panel_w, int groups_panel_h,
                                         int content_x, int scroll_y, int scroll_w, int scroll_h)
{
  Rectangle timing_events_rect = { .x = groups_panel_x, .y = groups_panel_y, .width = groups_panel_w, .height = groups_panel_h };
  Rectangle scissoring_rect = GetCollisionRec(timing_events_rect, (Rectangle){content_x, scroll_y, scroll_w, scroll_h});
  if (scissoring_rect.width == 0 || scissoring_rect.height == 0) return;

  int total_height = timing_groups->size * 30;
  GuiScrollPanel(timing_events_rect, "[ TIMINGS GROUPS ]",
                 (Rectangle){0, 0, scroll_w - 28, total_height},
                 &events_panel->tg_scroll, &events_panel->tg_scroll_rect);

  int height_top_bar_offset = (int)Clamp(timing_events_rect.y - scissoring_rect.y + 24.0f, 0.0f, 24.0f);
  if (scissoring_rect.height - height_top_bar_offset - 14 <= 0) return;
  BeginScissorMode(scissoring_rect.x, scissoring_rect.y + height_top_bar_offset,
                   scissoring_rect.width + 8, scissoring_rect.height - height_top_bar_offset - 15);

  // TODO: Editable values (self-harm)
  int timings_panel_y = groups_panel_y + 24 + events_panel->tg_scroll.y;
  int scroll_bar_offset = total_height < timing_events_rect.height ? 0 : 12;
  if (timing_groups->size > 0)
  {
    GuiDummyRec((Rectangle){groups_panel_x + 1, timings_panel_y + 30*events_panel->curr_tg, groups_panel_w - scroll_bar_offset - 2, 30}, NULL);
    for (int i = 0; i < timing_groups->size - 1; i++)
    {
      GuiLine((Rectangle){groups_panel_x, timings_panel_y + 30*i + 15, groups_panel_w - scroll_bar_offset - 1, 30}, NULL);
    }
    for (int i = 0; i < timing_groups->size; i++)
    {
      ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, i);
      if(GuiButton((Rectangle){groups_panel_x + 3, timings_panel_y + 30*i + 3, 25, 25},
               TextFormat("%d", i)))
      {
        events_panel->curr_tg = i;
      }
      GuiLabel((Rectangle){groups_panel_x + 8 + 24, timings_panel_y + 30*i, groups_panel_w - 28, 30},
               TextFormat("%s", tg->props.name[0] != '\0' ? tg->props.name : "-"));
    }
  }
  EndScissorMode();
}

static void draw_timings_panel_list(ChartTimingGroup *tg, WindowInst *window_inst, EventsPanel *events_panel,
                                    int events_panel_x, int events_panel_y, int events_panel_w, int events_panel_h,
                                    int content_x, int scroll_y, int scroll_w, int scroll_h)
{
  Rectangle timing_events_rect = { .x = events_panel_x, .y = events_panel_y, .width = events_panel_w, .height = events_panel_h - 30 };
  Rectangle scissoring_rect = GetCollisionRec(timing_events_rect, window_inst_get_rect(window_inst));
  if (scissoring_rect.width == 0 || scissoring_rect.height == 0) return;

  int total_height = tg != NULL ? tg->timing_events.size * 30 : 0;
  GuiScrollPanel(timing_events_rect, "[ TIMINGS ]",
                 (Rectangle){0, 0, scroll_w - 28, total_height},
                 &events_panel->event_scroll, &events_panel->event_scroll_rect);

  int height_top_bar_offset = (int)Clamp(timing_events_rect.y - scissoring_rect.y + 24.0f, 0.0f, 24.0f);
  if (scissoring_rect.height - height_top_bar_offset - 14 <= 0) return;
  BeginScissorMode(scissoring_rect.x, scissoring_rect.y + height_top_bar_offset,
                   scissoring_rect.width + 8, scissoring_rect.height - height_top_bar_offset - 15);

  // TODO: Editable values (self-harm)
  int timings_panel_y = events_panel_y + 24 + events_panel->event_scroll.y;
  int scroll_bar_offset = total_height < timing_events_rect.height ? 0 : 12;
  if (tg != NULL)
  {
    for (int i = 0; i < tg->timing_events.size - 1; i++)
      GuiLine((Rectangle){events_panel_x + 8, timings_panel_y + 30*i + 15, events_panel_w - scroll_bar_offset - 16, 30}, NULL);
    for (int i = 0; i < tg->timing_events.size; i++)
    {
      TimingEvent *event = (TimingEvent *)list_get(&tg->timing_events, i);
      char timing_str[16]; sprintf(timing_str, "%d", event->timing);
      char bpm_str[16];    sprintf(bpm_str,  "%.3f", event->bpm);
      char div_str[16];    sprintf(div_str,  "%.3f", event->divisor);
      GuiLabel((Rectangle){events_panel_x + 8 + 104*0, timings_panel_y + 30*i, events_panel_w - 28, 30}, timing_str);
      GuiLabel((Rectangle){events_panel_x + 8 + 104*1, timings_panel_y + 30*i, events_panel_w - 28, 30}, bpm_str);
      GuiLabel((Rectangle){events_panel_x + 8 + 104*2, timings_panel_y + 30*i, events_panel_w - 28, 30}, div_str);
    }
  }
  EndScissorMode();
}

static size_t get_builtin_sc_size(enum BuiltinSCOption option, ChartTimingGroup *tg)
{
  switch(option)
  {
    case EVENTS_PANEL_HIDEGROUP:     return tg->hidegroup_channel.keyframes.size;
    case EVENTS_PANEL_GROUPALPHA:    return tg->groupalpha_channel.keyframes.size;
    case EVENTS_PANEL_ENWIDENCAMERA: return tg->enwidencamera_channel.keyframes.size;
    case EVENTS_PANEL_ENWIDENLANES:  return tg->enwidenlanes_channel.keyframes.size;
  }
}

static void draw_events_panel_list(ChartTimingGroup *tg, enum BuiltinSCOption sc_option, WindowInst *window_inst, EventsPanel *events_panel,
                                   int events_panel_x, int events_panel_y, int events_panel_w, int events_panel_h,
                                   int content_x, int scroll_y, int scroll_w, int scroll_h)
{
  Rectangle timing_events_rect = { .x = events_panel_x, .y = events_panel_y, .width = events_panel_w, .height = events_panel_h - 30};
  Rectangle scissoring_rect = GetCollisionRec(timing_events_rect, window_inst_get_rect(window_inst));
  if (scissoring_rect.width == 0 || scissoring_rect.height == 0) return;

  int total_height = tg != NULL ? get_builtin_sc_size(events_panel->scenecontrol.option, tg) * 30 : 0;
  char result[16]; builtin_sc_option_get_label_text(events_panel->scenecontrol.option, result, 256);
  GuiScrollPanel(timing_events_rect, TextFormat("[ EVENTS - %s ]", result),
                 (Rectangle){0, 0, scroll_w - 28, total_height},
                 &events_panel->event_scroll, &events_panel->event_scroll_rect);

  int height_top_bar_offset = (int)Clamp(timing_events_rect.y - scissoring_rect.y + 24.0f, 0.0f, 24.0f);
  if (scissoring_rect.height - height_top_bar_offset - 14 <= 0) return;
  BeginScissorMode(scissoring_rect.x, scissoring_rect.y + height_top_bar_offset,
                   scissoring_rect.width + 8, scissoring_rect.height - height_top_bar_offset - 15);

  // TODO: Editable values (self-harm)
  // TODO: start timing + duration instead of keyframe (self-harm)
  int timings_panel_y = events_panel_y + 24 + events_panel->event_scroll.y;
  if (tg != NULL)
  {
    GuiLabel((Rectangle){events_panel_x + 8, timings_panel_y, events_panel_w - 28, 30}, "soon tm");
    // switch(sc_option)
    // {
    //   case EVENTS_PANEL_HIDEGROUP:
    //     {
    //       for (int i = 0; i < tg->hidegroup_channel.keyframes.size - 1; i++)
    //         GuiLine((Rectangle){events_panel_x + 8, timings_panel_y + 30*i + 15, events_panel_w - 28, 30}, NULL);
    //       for (int i = 0; i < tg->hidegroup_channel.keyframes.size; i++)
    //       {
    //         ValueKeyframe *kf = (ValueKeyframe *)list_get(&tg->hidegroup_channel.keyframes, i);
    //         char timing_str[16]; sprintf(timing_str, "%d", kf->start_timing);
    //         GuiLabel((Rectangle){events_panel_x + 8 + 104*0, timings_panel_y + 30*i, events_panel_w - 28, 30}, timing_str);
    //       }
    //       break;
    //     }
    //   case EVENTS_PANEL_GROUPALPHA:
    //     {
    //       for (int i = 0; i < tg->groupalpha_channel.keyframes.size - 1; i++)
    //         GuiLine((Rectangle){events_panel_x + 8, timings_panel_y + 30*i + 15, events_panel_w - 28, 30}, NULL);
    //       for (int i = 0; i < tg->groupalpha_channel.keyframes.size; i++)
    //       {
    //         ValueKeyframe *kf = (ValueKeyframe *)list_get(&tg->groupalpha_channel.keyframes, i);
    //         char timing_str[16]; sprintf(timing_str, "%d", kf->start_timing);
    //         GuiLabel((Rectangle){events_panel_x + 8 + 104*0, timings_panel_y + 30*i, events_panel_w - 28, 30}, timing_str);
    //       }
    //       break;
    //     }
    //   case EVENTS_PANEL_ENWIDENCAMERA:
    //     {
    //       for (int i = 0; i < tg->enwidencamera_channel.keyframes.size - 1; i++)
    //         GuiLine((Rectangle){events_panel_x + 8, timings_panel_y + 30*i + 15, events_panel_w - 28, 30}, NULL);
    //       for (int i = 0; i < tg->enwidencamera_channel.keyframes.size; i++)
    //       {
    //         ValueKeyframe *kf = (ValueKeyframe *)list_get(&tg->enwidencamera_channel.keyframes, i);
    //         char timing_str[16]; sprintf(timing_str, "%d", kf->start_timing);
    //         GuiLabel((Rectangle){events_panel_x + 8 + 104*0, timings_panel_y + 30*i, events_panel_w - 28, 30}, timing_str);
    //       }
    //       break;
    //     }
    //   case EVENTS_PANEL_ENWIDENLANES:
    //     {
    //       for (int i = 0; i < tg->enwidenlanes_channel.keyframes.size - 1; i++)
    //         GuiLine((Rectangle){events_panel_x + 8, timings_panel_y + 30*i + 15, events_panel_w - 28, 30}, NULL);
    //       for (int i = 0; i < tg->enwidenlanes_channel.keyframes.size; i++)
    //       {
    //         ValueKeyframe *kf = (ValueKeyframe *)list_get(&tg->enwidenlanes_channel.keyframes, i);
    //         char timing_str[16]; sprintf(timing_str, "%d", kf->start_timing);
    //         GuiLabel((Rectangle){events_panel_x + 8 + 104*0, timings_panel_y + 30*i, events_panel_w - 28, 30}, timing_str);
    //       }
    //       break;
    //     }
    // }
  }
  EndScissorMode();
}

void events_panel_draw(WindowInst* window_inst, EventsPanel *events_panel, bool is_chart_initiailzed, Shader sdf_shader, List *timing_groups)
{
  int content_x = window_inst->x + 8;
  int toggle_y  = (int)window_inst->y + 32;
  int scroll_y  = toggle_y + 32;
  int scroll_w  = (int)window_inst->width  - 16;
  int scroll_h  = (int)window_inst->height - (scroll_y - (int)window_inst->y) - 8;

  if (!is_chart_initiailzed)
  {
    GuiLabel((Rectangle){content_x, scroll_y, scroll_w, 24}, "No chart currently loaded.");
    return;
  }

  BeginShaderMode(sdf_shader);

  int groups_panel_h = 228;
  int events_panel_h = 336;

  int input_field_h = 24;
  int input_field_gap = 30;
  int total_content_h = groups_panel_h + events_panel_h + 8*3;
  GuiScrollPanel((Rectangle){content_x, scroll_y, scroll_w, scroll_h},
                 NULL, (Rectangle){0, 0, scroll_w, total_content_h},
                 &events_panel->scroll, &events_panel->scroll_rect);

  BeginScissorMode(events_panel->scroll_rect.x, events_panel->scroll_rect.y, events_panel->scroll_rect.width + 8, events_panel->scroll_rect.height);

  int scroll_padding = (int)events_panel->scroll_rect.height >= total_content_h ? 16 : 28;

  // Information
  int groups_panel_x = content_x + 8   + (int)events_panel->scroll.x;
  int groups_panel_y = scroll_y  + 8*1 + (int)events_panel->scroll.y;
  int groups_panel_w = scroll_w  - scroll_padding;
  draw_timing_groups_panel_list(timing_groups, window_inst, events_panel,
                                groups_panel_x, groups_panel_y, groups_panel_w, groups_panel_h,
                                content_x, scroll_y, scroll_w, scroll_h);
  BeginScissorMode(events_panel->scroll_rect.x, events_panel->scroll_rect.y,
                   events_panel->scroll_rect.width + 8, events_panel->scroll_rect.height);

  // Events
  int events_panel_x = content_x + 8   + (int)events_panel->scroll.x;
  int events_panel_y = scroll_y  + 8*2 + (int)events_panel->scroll.y + groups_panel_h;
  int events_panel_w = scroll_w  - scroll_padding;
  rgui_selectinput_toggles(&events_panel->timings_events, (Rectangle){events_panel_x, events_panel_y, 56, 24});

  events_panel_y += 24 + 8;
  ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, events_panel->curr_tg);
  switch (events_panel->timings_events.option)
  {
    case EVENTS_PANEL_TIMINGS:
    {
      draw_timings_panel_list(tg, window_inst, events_panel,
                              events_panel_x, events_panel_y, events_panel_w, events_panel_h,
                              content_x, scroll_y, scroll_w, scroll_h);
      break;
    }
    case EVENTS_PANEL_EVENTS:
    {
      draw_events_panel_list(tg, events_panel->scenecontrol.option, window_inst, events_panel,
                             events_panel_x, events_panel_y, events_panel_w, events_panel_h,
                             content_x, scroll_y, scroll_w, scroll_h);
      break;
    }
  }
  BeginScissorMode(events_panel->scroll_rect.x, events_panel->scroll_rect.y,
                   events_panel->scroll_rect.width + 8, events_panel->scroll_rect.height);

  if (events_panel->timings_events.option == EVENTS_PANEL_EVENTS)
    rgui_selectinput_dropdown(&events_panel->scenecontrol,
                              (Rectangle){events_panel_x + events_panel_w - 120, events_panel_y - 24 - 8, 120, 24});

  EndScissorMode();
  EndShaderMode();
}

void events_panel_apply_chart(EventsPanel *events_panel)
{
  events_panel->curr_tg = 0;
}
