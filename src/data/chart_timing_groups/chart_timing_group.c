#include <stdio.h>
#include "chart_timing_group.h"
#include "data/custom_types/dynamic_list.h"
#include "data/gameplay_events/gameplay_events.h"
#include "render/mesh_renderable.h"

TGPropTypes determine_tg_prop(char *str)
{
  if (strstr(str, (const char*)"noinput" ) != NULL ||
      strstr(str, (const char*)"noinoput") != NULL) return NO_INPUT;
  if (strstr(str, (const char*)"noclip"  ) != NULL) return NO_CLIP;
  if (strstr(str, (const char*)"noarccap") != NULL) return NO_ARCCAP;
  if (strstr(str, (const char*)"noshadow") != NULL) return NO_SHADOW;
  if (strstr(str, (const char*)"noheightindicator") != NULL) return NO_HEIGHT_INDICATOR;
  return NO_TG_PROP;
}

ChartTimingGroup timing_group_init()
{
  ChartTimingGroup tg = { 0 };
  List t_events; list_init(&t_events, sizeof(TimingEvent)); tg.timing_events = t_events;
  List taps    ; list_init(&taps, sizeof(Tap))            ; tg.taps = taps;
  List holds   ; list_init(&holds, sizeof(Hold))          ; tg.holds = holds;
  List arcs    ; list_init(&arcs, sizeof(Arc))            ; tg.arcs = arcs;
  List arctaps ; list_init(&arctaps, sizeof(ArcTap))      ; tg.arctaps = arctaps;

  List tap_fps   ; list_init(&tap_fps, sizeof(TapFP))      ; tg.tap_fps = tap_fps;
  List arctap_fps; list_init(&arctap_fps, sizeof(ArcTapFP)); tg.arctap_fps = arctap_fps;

  itv_tree_init(&tg.holds_tree, double_compare_asc);

  List arc_segments; list_init(&arc_segments, sizeof(ArcSegment)); tg.arc_segments = arc_segments;
  itv_tree_init(&tg.arc_segments_tree, double_compare_asc);

  List beatlines; list_init(&beatlines, sizeof(BeatLine)); tg.beatlines = beatlines;
  return tg;
}

void timing_group_print(ChartTimingGroup *tg)
{
  printf("Timing group %d:\n", tg->value);
  list_print(&tg->timing_events, timing_event_print, "Timing event");
  list_print(&tg->taps, tap_print, "Taps");
  list_print(&tg->holds, hold_print, "Holds");
  list_print(&tg->arcs, arc_print, "Arcs");
  list_print(&tg->beatlines, beatline_print, "Beat lines");
}

void timing_group_unload(ChartTimingGroup *tg)
{
  for (size_t i = 0; i < tg->taps.size; i++)
  {
    Tap *tap = (Tap *)list_get(&tg->taps, i);
    list_free(&tap->connector_x);
    list_free(&tap->connector_y);
  }

  for (size_t i = 0; i < tg->arcs.size; i++)
  {
    Arc *arc = (Arc *)list_get(&tg->arcs, i);
    list_free(&arc->arctaps);
  }

  list_free(&tg->taps);
  list_free(&tg->holds);
  list_free(&tg->arcs);
  list_free(&tg->arctaps);
  list_free(&tg->timing_events);

  list_free(&tg->tap_fps);
  list_free(&tg->arctap_fps);

  for (size_t i = 0; i < tg->arc_segments.size; i++)
  {
    ArcSegment *arc_segment = (ArcSegment *)list_get(&tg->arc_segments, i);
    renderable_unload(&arc_segment->mesh_r);
    renderable_unload(&arc_segment->shadow_r);
  }
  itv_tree_free(&tg->holds_tree);

  list_free(&tg->arc_segments);
  itv_tree_free(&tg->arc_segments_tree);

  list_free(&tg->beatlines);
}

static int update_tg_props(char *token, void *user)
{
  ChartTimingGroup *tg = (ChartTimingGroup *)user;
  switch (determine_tg_prop(token))
  {
    case NO_INPUT:  tg->props.no_input  = true; break;
    case NO_CLIP:   tg->props.no_clip   = true; break;
    case NO_ARCCAP: tg->props.no_arccap = true; break;
    case NO_SHADOW: tg->props.no_shadow = true; break;
    case NO_HEIGHT_INDICATOR: tg->props.no_height_indicator = true; break;
    case NO_TG_PROP: break;
  };

  return 1;
}

void parse_tg_props(const char *line, ChartTimingGroup *tg)
{
  const char *lb = strchr(line, '('); if (!lb) return;
  const char *rb = strchr(lb  , ')'); if (!rb) return;

  size_t len = (size_t)(rb - lb - 1);
  char *copy = malloc(len + 1);
  if (!copy) return;

  memcpy(copy, lb + 1, len);
  copy[len] = '\0';
  split(copy, ',', update_tg_props, tg);
  free(copy);
}
