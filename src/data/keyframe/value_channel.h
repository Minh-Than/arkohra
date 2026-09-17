#ifndef VALUE_CHANNEL_H
#define VALUE_CHANNEL_H

#include "data/custom_types/dynamic_list.h"
#include "data/keyframe/easings.h"

typedef struct {
  float prev_value, next_value;
  int start_timing, end_timing;
  EasingType easing;
} ValueKeyframe;

void value_kf_print(ValueKeyframe kf);
int value_kf_compare_start_timing_asc(const void *a, const void *b);

typedef struct {
  List keyframes;
  float current_value;
} ValueChannel;

ValueChannel value_channel_init();
int value_channel_interpolate(ValueChannel *channel, int timing);
void value_channel_print(ValueChannel *channel);
void value_channel_unload(ValueChannel *channel);

#endif // VALUE_CHANNEL_H
