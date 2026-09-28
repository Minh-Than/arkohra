#ifndef TIMING_EVENT_H
#define TIMING_EVENT_H

#include "data/custom_types/dynamic_list.h"

typedef struct
{
  double fp;
  float  bpm, divisor;
  int   timing, timing_group;
  bool  is_selected;
} TimingEvent;

void timing_event_print(const void *elem);
void timing_event_parse_aff(List *timing_events, const char* line, int *current_tg);
int timing_event_compare_timing_asc(const void *a, const void *b);

#endif // TIMING_EVENT_H
