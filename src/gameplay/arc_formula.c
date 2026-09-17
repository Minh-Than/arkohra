#if defined(__x86_64__) || defined(__i386__)
  #include <immintrin.h>
#elif defined(__aarch64__) || defined(__arm__)
  #include <arm_neon.h>
#endif

#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include "constants.h"
#include "arc_formula.h"
#include "raymath.h"
#include "data/custom_types/dynamic_list.h"

float lane_to_world_x(float lane)
{
  return (LANE_WIDTH * lane) + (-LANE_WIDTH * 2.5f);
}

double z_to_floor_position(double z, float base_bpm, float scroll_speed)
{
  float final_scroll_speed = fminf(fmaxf(MINIMUM_SCROLL_SPEED, scroll_speed), MAXIMUM_SCROLL_SPEED);
  return z * base_bpm * -32 / final_scroll_speed;
}

double floor_position_to_z(double fp, float base_bpm, float scroll_speed)
{
  float final_scroll_speed = fminf(fmaxf(MINIMUM_SCROLL_SPEED, scroll_speed), MAXIMUM_SCROLL_SPEED);
  return fp / base_bpm / -32 * final_scroll_speed;
}

#if defined(__x86_64__) || defined(__i386__)
void batch_fp_to_z(double *fp_list, double *out, int count, float base_bpm, float scroll_speed)
{
  __m128d base_bpm_v     = _mm_set1_pd(base_bpm);
  __m128d constant_v     = _mm_set1_pd(-32.0);
  __m128d scroll_speed_v = _mm_set1_pd(scroll_speed);

  int i = 0;
  for (; i + 2 <= count; i += 2)
  {
    __m128d fp = _mm_loadu_pd(&fp_list[i]);
    __m128d z  = _mm_div_pd(fp, base_bpm_v);
    z          = _mm_div_pd(z, constant_v);
    z          = _mm_mul_pd(z, scroll_speed_v);
    _mm_storeu_pd(&out[i], z);
  }
  for (; i < count; i++)
    out[i] = floor_position_to_z(fp_list[i], base_bpm, scroll_speed);
}
#elif defined(__aarch64__)
void batch_fp_to_z(double *fp_list, double *out, int count, float base_bpm, float scroll_speed)
{
  float64x2_t base_bpm_v     = vdupq_n_f64(base_bpm);
  float64x2_t constant_v     = vdupq_n_f64(-32.0);
  float64x2_t scroll_speed_v = vdupq_n_f64(scroll_speed);

  int i = 0;
  for (; i + 2 <= count; i += 2)
  {
    float64x2_t fp = vld1q_f64(&fp_list[i]);
    float64x2_t z  = vdivq_f64(fp, base_bpm_v);
    z              = vdivq_f64(z, constant_v);
    z              = vmulq_f64(z, scroll_speed_v);
    vst1q_f64(&out[i], z);
  }
  for (; i < count; i++)
    out[i] = floor_position_to_z(fp_list[i], base_bpm, scroll_speed);
}
#else
void batch_fp_to_z(double *fp_list, double *out, int count, float base_bpm, float scroll_speed)
{
  for (int i = 0; i < count; i++)
    out[i] = floor_position_to_z(fp_list[i], base_bpm, scroll_speed);
}
#endif

void recalculate_floor_position(ChartTimingGroup *timing_group)
{
  List *timing_events = &timing_group->timing_events;
  if (timing_events->size == 0) return;

  double fp = 0;
  for (int i = 0; i < timing_events->size - 1; i++)
  {
    TimingEvent *curr = (TimingEvent *)list_get(timing_events, i);
    TimingEvent *next = (TimingEvent *)list_get(timing_events, i + 1);
    curr->fp = fp;
    fp += (double)(next->timing - curr->timing) * curr->bpm;
  }

  TimingEvent *event = (TimingEvent *)list_get(timing_events, timing_events->size - 1);
  event->fp = fp;
}

TimingEvent *get_event_at(List *timing_events, int timing)
{
  if (timing_events->size == 0) return NULL;
  TimingEvent te = { .timing = timing };
  int index = bisect_right(timing_events, &te, timing_event_compare_timing_asc) - 1;
  index = fmaxf(index, 0.0f);
  return (TimingEvent *)list_get(timing_events, index);
}

double get_floor_position(List *timing_events, int timing)
{
  TimingEvent *note = get_event_at(timing_events, timing);
  if (note == NULL) return 0.0;
  double baseFloorPosition = note->fp;
  return baseFloorPosition + ((double)note->bpm * (timing - note->timing));
}

double get_fp_from_current(List *timing_events, int timing, int current_timing)
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

float arc_world_x_at(int timing, Arc *arc)
{
  if (arc->end_timing == arc->start_timing) return arc_x_to_world(timing <= arc->start_timing ? arc->x1 : arc->x2);

  float p = Clamp((float)(timing - arc->start_timing) / (arc->end_timing - arc->start_timing), 0, 1);
  return arc_x_to_world(_x(arc->x1, arc->x2, p, arc->type));
}

float arc_world_y_at(int timing, Arc *arc)
{
  if (arc->end_timing == arc->start_timing) return arc_y_to_world(timing <= arc->start_timing ? arc->y1 : arc->y2);

  float p = Clamp((float)(timing - arc->start_timing) / (arc->end_timing - arc->start_timing), 0, 1);
  return arc_y_to_world(_y(arc->y1, arc->y2, p, arc->type));
}

float calculate_arc_segment_length(int duration, float arc_resolution)
{
  if(arc_resolution < MINIMUM_ARC_RES) return duration;
  float final_resolution = fminf(MAXIMUM_ARC_RES, arc_resolution);
  float length = ARC_SEGMENT_LENGTH / final_resolution;
  return duration < 1000 ? length : length * 2;
}
