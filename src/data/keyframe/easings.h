#ifndef EASINGS_H
#define EASINGS_H

typedef enum {
  E_LINEAR,
  E_STEP_START,  E_STEP_END,
  E_OUT_SINE,    E_INOUT_SINE,    E_IN_SINE,
  E_OUT_QUART,   E_INOUT_QUART,   E_IN_QUART,
  E_OUT_QUAD,    E_INOUT_QUAD,    E_IN_QUAD,
  E_OUT_QUINT,   E_INOUT_QUINT,   E_IN_QUINT,
  E_OUT_EXPO,    E_INOUT_EXPO,    E_IN_EXPO,
  E_OUT_ELASTIC, E_INOUT_ELASTIC, E_IN_ELASTIC,
  E_OUT_CUBIC,   E_INOUT_CUBIC,   E_IN_CUBIC,
  E_OUT_CIRC,    E_INOUT_CIRC,    E_IN_CIRC,
  E_OUT_BOUNCE,  E_INOUT_BOUNCE,  E_IN_BOUNCE,
  E_OUT_BACK,    E_INOUT_BACK,    E_IN_BACK,
} EasingType;

float easing_get_unit(EasingType easing, float t);
float easing_interpolate(EasingType easing, int timing, int start_timing, int end_timing, float from, float to);

#endif // EASINGS_H
