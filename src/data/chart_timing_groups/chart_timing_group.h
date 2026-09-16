#ifndef CHART_TIMING_GROUP_H
#define CHART_TIMING_GROUP_H

#include <stdbool.h>
#include "./data/custom_types/custom_types.h"
#include "data/keyframe/value_channel.h"

typedef enum {
  TG_NAME,
  ARC_RESOLUTION,
  NO_INPUT,
  NO_CLIP,
  NO_ARCCAP,
  NO_HEIGHT_INDICATOR,
  NO_SHADOW,
  NO_TG_PROP
} TGPropTypes;

TGPropTypes determine_tg_prop(char *str);

typedef struct {
  char name[256];
  float arc_res;
  bool no_input, no_clip, no_arccap, no_height_indicator, no_shadow;
} TimingGroupProps;

void tg_props_print(TimingGroupProps *tg_props);

typedef struct {
  int value;
  TimingGroupProps props;
  List taps; List tap_fps;
  List holds; ItvTree holds_tree;
  List arcs; List arc_segments; ItvTree arc_segments_tree;
  List arctaps; List arctap_fps;
  List timing_events;
  List beatlines;
  ValueChannel hidegroup_channel;

} ChartTimingGroup;

ChartTimingGroup timing_group_init();
void timing_group_info_print(ChartTimingGroup *tg);
void timing_group_unload(ChartTimingGroup *chart_tg);
void parse_tg_props(const char *line, ChartTimingGroup *tg);

#endif // CHART_TIMING_GROUP_H

