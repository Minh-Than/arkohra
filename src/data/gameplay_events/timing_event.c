#include <stdio.h>
#include "timing_event.h"

void timing_event_print(const void *elem)
{
  const TimingEvent *t_event = (const TimingEvent *)elem;
  printf("timing(%d,%.2f,%.2f)", t_event->timing, t_event->bpm, t_event->divisor);
}

void timing_event_parse_aff(List *timing_events, const char* line, int *current_tg)
{
  int timing;
  float bpm, divisor;
  int matched = sscanf(line, "timing(%d,%f,%f);", &timing, &bpm, &divisor);
  if (matched == 3)
  {
    TimingEvent t_event = { .fp = 0, .bpm = bpm, .divisor = divisor, .timing = timing, .timing_group = *current_tg };
    list_push(timing_events, &t_event);
  }
}

int timing_event_compare_timing_asc(const void *a, const void *b)
{
  const TimingEvent *tap_a = (const TimingEvent *) a;
  const TimingEvent *tap_b = (const TimingEvent *) b;
  return tap_a->timing - tap_b->timing;
}
