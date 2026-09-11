#ifndef CHART_TIMING_GROUP_H
#define CHART_TIMING_GROUP_H

#include <stdbool.h>
#include "./data/custom_types/custom_types.h"

// TODO: name and arcresolution later
typedef enum {
  // NAME,
  NO_INPUT,
  NO_CLIP,
  NO_ARCCAP,
  NO_HEIGHT_INDICATOR,
  NO_SHADOW,
  NO_TG_PROP
} TGPropTypes;

TGPropTypes determine_tg_prop(char *str);

typedef struct {
  bool no_input, no_clip, no_arccap, no_height_indicator, no_shadow;
  // char name[256];
} TimingGroupProps;

typedef struct {
  int value;
  TimingGroupProps props;
  List taps; List tap_fps;
  List holds; ItvTree holds_tree;
  List arcs; List arc_segments; ItvTree arc_segments_tree;
  List arctaps; List arctap_fps;
  List timing_events;
  List beatlines;

} ChartTimingGroup;

ChartTimingGroup timing_group_init();
void timing_group_print(ChartTimingGroup *chart_tg);
void timing_group_unload(ChartTimingGroup *chart_tg);
void parse_tg_props(const char *line, ChartTimingGroup *tg);

#endif // CHART_TIMING_GROUP_H

