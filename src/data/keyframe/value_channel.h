#ifndef VALUE_CHANNEL_H
#define VALUE_CHANNEL_H

#include "data/custom_types/dynamic_list.h"
#include "data/keyframe/easings.h"

typedef struct {
  float value;
  int timing;
  EasingType easing;
} ValueKeyframe;

int value_kf_compare_timing_asc(const void *a, const void *b);

typedef struct {
  List keyframes;
} ValueChannel;

ValueChannel value_channel_init();
float value_channel_interpolate(ValueChannel *channel, int timing);
void value_channel_unload(ValueChannel *channel);

#endif // VALUE_CHANNEL_H
