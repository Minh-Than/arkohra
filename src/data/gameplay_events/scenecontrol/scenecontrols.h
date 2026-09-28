#ifndef SCENECONTROLS_H
#define SCENECONTROLS_H

#include "data/chart_timing_groups/chart_timing_group.h"

typedef enum {
  SC_HIDEGROUP,
  SC_GROUPALPHA,
  SC_ENWIDENCAMERA,
  SC_ENWIDENLANES,
  SC_OTHERS,
  SC_NONE
} SCType;

SCType scenecontrol_determine_type(char *str_name);
void scenecontrol_parse_aff(ChartTimingGroup *tg, const char* line);

#endif // SCENECONTROLS_H
