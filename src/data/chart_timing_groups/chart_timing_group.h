#ifndef CHART_TIMING_GROUP_H
#define CHART_TIMING_GROUP_H

#include <stdbool.h>
#include "./data/custom_types/custom_types.h"

typedef struct
{
  List taps; List tap_fps;
  List holds; ItvTree holds_tree;
  List arcs; List arc_segments; ItvTree arc_segments_tree;
  List arctaps; List arctap_fps;
  List timing_events;

} ChartTimingGroup;

ChartTimingGroup timing_group_init();
void timing_group_print(ChartTimingGroup (*chart_data));
void timing_group_unload(ChartTimingGroup (*chart_data));

#endif // CHART_TIMING_GROUP_H

