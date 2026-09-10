#include <stdio.h>
#include "chart_timing_group.h"
#include "data/custom_types/dynamic_list.h"
#include "data/gameplay_events/gameplay_events.h"
#include "render/mesh_renderable.h"

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
  // list_print(&tg->timing_events, timing_event_print, "Timing event");
  // list_print(&tg->taps, tap_print, "Taps");
  // list_print(&tg->holds, hold_print, "Holds");
  // list_print(&tg->arcs, arc_print, "Arcs");
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
