#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include "constants.h"
#include "arc_formula.h"
#include "raymath.h"

float lane_to_world_x(float lane)
{
  return (LANE_WIDTH * lane) + (-LANE_WIDTH * 2.5f);
}

float floor_position_to_z(double fp, float base_bpm, float scroll_speed)
{
  return (float)(fp / base_bpm / -32 * scroll_speed);
}

void recalculate_floor_position(ChartTimingGroup *timing_group)
{
  List *timing_events = &timing_group->timing_events;
  if (timing_events->size == 0) return;

  float fp = 0;
  for (int i = 0; i < timing_events->size - 1; i++)
  {
    TimingEvent *curr = (TimingEvent *)list_get(timing_events, i);
    TimingEvent *next = (TimingEvent *)list_get(timing_events, i + 1);
    curr->fp = fp;
    fp += (next->timing - curr->timing) * curr->bpm;
  }

  TimingEvent *event = (TimingEvent *)list_get(timing_events, timing_events->size - 1);
  event->fp = fp;
}

TimingEvent *get_event_at(List *timing_events, int timing)
{
  TimingEvent te = { .timing = timing };
  int index = bisect_right(timing_events, &te, timing_event_compare_timing_asc) - 1;
  index = fmaxf(index, 0.0f);
  return (TimingEvent *)list_get(timing_events, index);
}

float get_floor_position(List *timing_events, int timing)
{
  TimingEvent *note = get_event_at(timing_events, timing);
  float baseFloorPosition = note->fp;
  return baseFloorPosition + (note->bpm * (timing - note->timing));
}

float get_fp_from_current(List *timing_events, int timing, int current_timing)
{
  double note_fp = get_floor_position(timing_events, timing);
  double curr_fp = get_floor_position(timing_events, current_timing);
  return note_fp - curr_fp;
}

static float _s(float start, float end, float t)
{
  return ((1 - t) * start) + (end * t);
}

static float _o(float start, float end, float t)
{
  return start + ((end - start) * (1 - cosf(1.5707963f * t)));
}

static float _i(float start, float end, float t)
{
  return start + ((end - start) * sinf(1.5707963f * t));
}

static float _b(float start, float end, float t)
{
  float o = 1 - t;
  return (powf(o, 3) * start)
       + (3 * powf(o, 2) * t * start)
       + (3 * o * powf(t, 2) * end)
       + (powf(t, 3) * end);
}

float _x(float start, float end, float t, ArcType type)
{
  switch (type)
  {
      default:
      case S:
        return _s(start, end, t);
      case B:
        return _b(start, end, t);
      case SI:
      case SISI:
      case SISO:
          return _i(start, end, t);
      case SO:
      case SOSI:
      case SOSO:
          return _o(start, end, t);
  }
}

float _y(float start, float end, float t, ArcType type)
{
  switch (type)
  {
      default:
      case S:
      case SI:
      case SO:
          return _s(start, end, t);
      case B:
          return _b(start, end, t);
      case SISI:
      case SOSI:
          return _i(start, end, t);
      case SISO:
      case SOSO:
          return _o(start, end, t);
  }
}

float arc_x_to_world(float x) { return (LANE_WIDTH * 2 * x) - LANE_WIDTH; }

float arc_y_to_world(float y) { return ARC_Y0 + ((ARC_Y1 - ARC_Y0) * y); }

float arc_world_x_at(int timing, Arc *arc, float fallback_x)
{
  if (arc->end_timing == arc->start_timing) return arc_x_to_world(fallback_x);

  float p = Clamp((float)(timing - arc->start_timing) / (arc->end_timing - arc->start_timing), 0, 1);
  return arc_x_to_world(_x(arc->x1, arc->x2, p, arc->type));
}

float arc_world_y_at(int timing, Arc *arc, float fallback_y)
{
  if (arc->end_timing == arc->start_timing) return arc_y_to_world(fallback_y);

  float p = Clamp((float)(timing - arc->start_timing) / (arc->end_timing - arc->start_timing), 0, 1);
  return arc_y_to_world(_y(arc->y1, arc->y2, p, arc->type));
}

float calculate_arc_segment_length(int duration, float arc_resolution)
{
  if(arc_resolution - 0.0f < 1e-6) return duration;
  float length = ARC_SEGMENT_LENGTH / arc_resolution;
  return duration < 1000 ? length : length * 2;
}
