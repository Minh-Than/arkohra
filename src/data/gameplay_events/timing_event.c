#include <stdio.h>
#include "timing_event.h"

void timing_event_print(const void *elem)
{
  const TimingEvent *t_event = (const TimingEvent *)elem;
  printf("timing(%d,%.2f,%.2f)", t_event->timing, t_event->bpm, t_event->divisor);
}


int timing_event_compare_timing_asc(const void *a, const void *b)
{
  const TimingEvent *tap_a = (const TimingEvent *) a;
  const TimingEvent *tap_b = (const TimingEvent *) b;
  return tap_a->timing - tap_b->timing;
}
