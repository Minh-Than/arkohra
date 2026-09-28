#include <math.h>
#include "easings.h"

// thks easings.net
static float ease_out_bounce(float t)
{
  const float n1 = 7.5625f;
  const float d1 = 2.75f;

  if (t < 1 / d1) return n1 * t * t;
  t -= 1.5 / d1;   if (t < 0.5 / d1)  return n1 * t * t + 0.75;
  t -= 0.75 / d1;  if (t < 0.25 / d1) return n1 * t * t + 0.9375;
  t -= 0.375 / d1; return n1 * t * t + 0.984375;
}

float easing_get_unit(EasingType easing, float t)
{
  switch (easing)
  {
    default:
    case E_LINEAR:        return t;
    case E_STEP_START:    return t <= 0.0f ? 0.0f : 1.0f;
    case E_STEP_END:      return t <= 1.0f ? 0.0f : 1.0f;
    case E_OUT_SINE:      return sinf(t * M_PI * 0.5f);
    case E_INOUT_SINE:    return -(cosf(t * M_PI) - 1.0f) * 0.5f;
    case E_IN_SINE:       return 1.0f - cosf(t * M_PI * 0.5f);
    case E_OUT_QUART:     return 1.0f - powf(1.0f - t, 4.0f);
    case E_INOUT_QUART:   return t < 0.5f ? 8.0f * t * t * t * t
                                          : 1.0f - powf(-2.0f * t + 2.0f, 4.0f) * 0.5f;
    case E_IN_QUART:      return t * t * t * t;
    case E_OUT_QUAD:      return 1.0f - (1.0f - t) * (1.0f - t);
    case E_INOUT_QUAD:    return t < 0.5f ? 2 * t * t
                                          : 1.0f - powf(-2.0f * t + 2.0f, 2.0f) * 0.5f;
    case E_IN_QUAD:       return t * t;
    case E_OUT_QUINT:     return 1.0f - powf(1.0f - t, 5.0f);
    case E_INOUT_QUINT:   return t < 0.5f ? 16.0f * t * t * t * t * t
                                          : 1.0f - powf(-2.0f * t + 2.0f, 5.0f) * 0.5f;
    case E_IN_QUINT:      return t * t * t * t * t;
    case E_OUT_EXPO:      return t >= 1.0f ? 1.0f
                                           : 1.0f - powf(2.0f, -10.0f * t);
    case E_INOUT_EXPO:    return t <= 0.0f ? 0.0f
                                           : t >= 1.0f ? 1.0f
                                                       : t < 0.5f ? powf(2.0f, 20.0f * t - 10.0f) * 0.5f
                                                                  : (2.0f - powf(2.0f, -20.0f * t + 10.0f)) * 0.5f;
    case E_IN_EXPO:       return t <= 0.0f ? 0.0f
                                           : powf(2.0f, 10 * t -10.0f);
    case E_OUT_ELASTIC:   return t <= 0.0f ? 0.0f
                                           : t >= 1.0f ? 1.0f
                                                       : powf(2.0f, -10.0f * t) * sinf((t * 10.0f - 0.75f) * ((2.0f * M_PI) / 3.0f)) + 1;
    case E_INOUT_ELASTIC: return t <= 0.0f ? 0.0f
                                           : t >= 1.0f ? 1.0f
                                                       : t < 0.5f ? -(powf(2.0f, 20.0f * t - 10.0f) * sinf((20.0f * t - 11.125f) * ((2.0f * M_PI) / 4.5))) * 0.5f
                                                                  : (powf(2.0f, -20.0f * t + 10.0f) * sinf((20.0f * t - 11.125f) * ((2.0f * M_PI) / 4.5))) * 0.5f + 1.0f;
    case E_IN_ELASTIC:    return t <= 0.0f ? 0.0f
                                           : t >= 1.0f ? 1.0f
                                                       : -powf(2.0f, 10.0f * t - 10.0f) * sinf((t * 10.0f- 10.75f) * ((2.0f * M_PI) / 3.0f));
    case E_OUT_CUBIC:     return 1.0f - powf(1.0f - t, 3.0f);
    case E_INOUT_CUBIC:   return t < 0.5f ? 4.0f * t * t * t
                                          : 1.0f - powf(-2.0f * t + 2.0f, 3.0f) * 0.5f;
    case E_IN_CUBIC:      return t * t * t;
    case E_OUT_CIRC:      return sqrtf(1.0f - powf(t - 1.0f, 2.0f));
    case E_INOUT_CIRC:    return t < 0.5f ? (1.0f - sqrtf(1.0f - powf(2.0f * t, 2.0f))) * 0.5f
                                          : (sqrtf(1.0f - powf(-2.0f * t + 2.0f, 2.0f)) + 1.0f) * 0.5f;
    case E_IN_CIRC:       return 1.0f - sqrtf(1.0f - powf(t, 2.0f));
    case E_OUT_BOUNCE:    return ease_out_bounce(t);
    case E_INOUT_BOUNCE:  return t < 0.5f ? (1.0f - ease_out_bounce(1.0f - 2.0f * t)) * 0.5f
                                          : (1.0f + ease_out_bounce(2.0f * t - 1.0f)) * 0.5f;
    case E_IN_BOUNCE:     return 1.0f - ease_out_bounce(1.0f - t);
    case E_OUT_BACK:      return 1.0f + 2.70158f * powf(t - 1.0f, 3.0f) + 1.70158f * powf(t - 1.0f, 2.0f);
    case E_INOUT_BACK:    {
                            const float c2 = 1.70158f * 1.525f;
                            return t < 0.5 ? (powf(2.0f * t, 2.0f) * ((c2 + 1.0f) * 2.0f * t - c2)) * 0.5f
                                           : (powf(2.0f * t - 2.0f, 2.0f) * ((c2 + 1.0f) * (t * 2.0f - 2.0f) + c2) + 2.0f) * 0.5f;
                          }
    case E_IN_BACK:       return 2.70158f * t * t * t - 1.70158f * t * t;
  }
}

float easing_interpolate(EasingType easing, int timing, int start_timing, int end_timing, float from, float to)
{
  if (fabsf(from - to) < 1e-6) return from;
  if (start_timing == end_timing) return to;
  const float unit_val = (float)(timing - start_timing) / (float)(end_timing - start_timing);
  return from + (to - from) * easing_get_unit(easing, unit_val);
}
