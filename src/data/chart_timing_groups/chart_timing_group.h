#ifndef CHART_TIMING_GROUP_H
#define CHART_TIMING_GROUP_H

#include <stdbool.h>
#include "./data/custom_types/custom_types.h"

typedef struct
{
  List taps;
  List holds;
  List arcs;
  List arctaps;
  List timing_events;
} ChartTimingGroup;

ChartTimingGroup timing_group_init();
void timing_group_print(ChartTimingGroup (*chart_data));
void timing_group_unload(ChartTimingGroup (*chart_data));

#endif // CHART_TIMING_GROUP_H

