#include <math.h>
#include <float.h>
#include "beatline.h"
#include "data/gameplay_events/timing_event.h"
#include "gameplay/arc_formula.h"
#include "raylib.h"

List beatline_generate(ChartTimingGroup *tg, int audio_length_ms, int audio_offset, Color color, float thickness)
{
  List beatlines; list_init(&beatlines, sizeof(BeatLine));
  List *timings = &tg->timing_events;

  // Before timing = 0
  {
    TimingEvent *first_event = (TimingEvent *)list_get(timings, 0);
    float start = -3000 - audio_offset;

    if (first_event->bpm <= 10000)
    {
      double delta = fabs((first_event->bpm * first_event->divisor) - 0.0f ) < 1e-6 ?
                     DBL_MAX : 60000.0 / fabs(first_event->bpm) * first_event->divisor;
      delta = fmax(delta, 1);
      if (delta > 0)
      {
        int count = 0;
        double timing = 0;
        while (timing >= start)
        {
          int t = (int)roundf(timing);
          BeatLine bl = { .color = color,
                          .fp = get_floor_position(timings, t),
                          .thickness = thickness,
                          .timing = t,
                          .timing_group = tg->value };
          list_push(&beatlines, &bl);
          count++;
          timing = -count * delta;
        }
      }
    }
  }

  // During audio
  for (int i = 0; i < timings->size - 1; i++)
  {
    TimingEvent *curr_event = (TimingEvent *)list_get(timings, i);
    int limit = ((TimingEvent *)list_get(timings, i + 1))->timing;

    if (curr_event->bpm > 10000) continue;

    double delta = fabs((curr_event->bpm * curr_event->divisor) - 0.0f ) < 1e-6 ?
                   DBL_MAX : 60000.0 / fabs(curr_event->bpm) * curr_event->divisor;
    delta = fmax(delta, 1);
    if (delta <= 0) continue;

    int count = 0;
    double timing = curr_event->timing;
    while (timing < limit)
    {
      int t = (int)roundf(timing);
      BeatLine bl = { .color = color,
                      .fp = get_floor_position(timings, t),
                      .thickness = thickness,
                      .timing = t,
                      .timing_group = tg->value };
      list_push(&beatlines, &bl);
      count++;
      timing = curr_event->timing + (delta * count);
    }
  }

  // Until audio ends
  {
    TimingEvent *last_event = (TimingEvent *)list_get(timings, timings->size - 1);
    float limit = audio_length_ms;

    if (last_event->bpm <= 10000)
    {
      double delta = fabs((last_event->bpm * last_event->divisor) - 0.0f ) < 1e-6 ?
                     DBL_MAX : 60000.0 / fabs(last_event->bpm) * last_event->divisor;
      delta = fmax(delta, 1);
      if (delta > 0)
      {
        int count = 0;
        double timing = last_event->timing;
        while (timing <= limit)
        {
          int t = (int)roundf(timing);
          BeatLine bl = { .color = color,
                          .fp = get_floor_position(timings, t),
                          .thickness = thickness,
                          .timing = t,
                          .timing_group = tg->value };
          list_push(&beatlines, &bl);
          count++;
          timing = last_event->timing + (delta * count);
        }
      }
    }
  }

  return beatlines;
}

int beatline_compare_fp_asc(const void *a, const void *b)
{
  const BeatLine *beatline_a = (const BeatLine *) a;
  const BeatLine *beatline_b = (const BeatLine *) b;
  double fp_a = beatline_a->fp;
  double fp_b = beatline_b->fp;
  if (fabs(fp_a - fp_b) > 1e-6 && fp_a < fp_b) return -1;
  if (fabs(fp_a - fp_b) > 1e-6 && fp_a > fp_b) return 1;
  return 0;
}

void beatline_print(const void* elem)
{
  const BeatLine *beatline = (const BeatLine *)elem;
  printf("beatline(%d, %d) FP: %.2lf", beatline->timing_group, beatline->timing, beatline->fp);
}
