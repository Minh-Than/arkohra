#ifndef VALUE_CHANNEL_H
#define VALUE_CHANNEL_H

#include "data/custom_types/dynamic_list.h"
#include "data/keyframe/easings.h"

typedef struct {
  float value;
  int timing;
  EasingType easing;
} ValueKeyframe;

typedef struct {
  List keyframes;
} ValueChannel;
void value_channel_init(ValueChannel *channel);

#endif // VALUE_CHANNEL_H
