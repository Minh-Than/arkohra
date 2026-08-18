#ifndef TIMING_EVENT_H
#define TIMING_EVENT_H

typedef struct
{
  float fp;
  float bpm, divisor;
  int   timing, timing_group;
  bool  is_selected;
} TimingEvent;

void timing_event_print(const void *elem);
int timing_event_compare_timing_asc(const void *a, const void *b);

#endif // TIMING_EVENT_H
