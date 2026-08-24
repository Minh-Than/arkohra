#include <stdio.h>
#include "chart_timing_group.h"
#include "data/custom_types/custom_types.h"
#include "data/gameplay_events/arc.h"
#include "data/gameplay_events/gameplay_events.h"
#include "data/gameplay_events/timing_event.h"
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

  List arc_segments; list_init(&arc_segments, sizeof(ArcSegment)); tg.arc_segments = arc_segments;
  return tg;
}

void timing_group_print(ChartTimingGroup *chart_data)
{
  printf("Timing group:\n");
  list_print(&chart_data->timing_events, timing_event_print, "Timing event");
  list_print(&chart_data->taps, tap_print, "Taps");
  list_print(&chart_data->holds, hold_print, "Holds");
  list_print(&chart_data->arcs, arc_print, "Arcs");
}

void timing_group_unload(ChartTimingGroup (*chart_data))
{
  for (size_t i = 0; i < chart_data->taps.size; i++)
  {
    Tap *tap = (Tap *)list_get(&chart_data->taps, i);
    list_free(&tap->connector_x);
    list_free(&tap->connector_y);
  }

  for (size_t i = 0; i < chart_data->arcs.size; i++)
  {
    Arc *arc = (Arc *)list_get(&chart_data->arcs, i);
    list_free(&arc->arctaps);
  }

  list_free(&chart_data->taps);
  list_free(&chart_data->holds);
  list_free(&chart_data->arcs);
  list_free(&chart_data->arctaps);
  list_free(&chart_data->timing_events);

  list_free(&chart_data->tap_fps);
  list_free(&chart_data->arctap_fps);

  for (size_t i = 0; i < chart_data->arc_segments.size; i++)
  {
    ArcSegment *arc_segment = (ArcSegment *)list_get(&chart_data->arc_segments, i);
    renderable_unload(&arc_segment->mesh_r);
    renderable_unload(&arc_segment->shadow_r);
  }
  list_free(&chart_data->arc_segments);
}
