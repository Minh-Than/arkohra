#include <stdlib.h>
#include "value_channel.h"
#include "data/custom_types/dynamic_list.h"
#include "data/keyframe/easings.h"

int value_kf_compare_timing_asc(const void *a, const void *b)
{
  const ValueKeyframe *kf_a = (const ValueKeyframe *) a;
  const ValueKeyframe *kf_b = (const ValueKeyframe *) b;
  return kf_a->timing - kf_b->timing;
}

ValueChannel value_channel_init()
{
  List keyframes; list_init(&keyframes, sizeof(ValueKeyframe));
  ValueChannel channel = { .keyframes = keyframes };
  return channel;
}

// Keyframes list HAS to be sorted
float value_channel_interpolate(ValueChannel *channel, int timing)
{
  if (channel == NULL) return 0.0f;

  List *kfs = &channel->keyframes;
  if(kfs->data == NULL || kfs->size == 0) return 0.0f;

  ValueKeyframe target_kf = { .timing = timing };
  int current_kf_idx = bisect_left(kfs, &target_kf, value_kf_compare_timing_asc);
  ValueKeyframe *curr_kf = (ValueKeyframe *)list_get(kfs, current_kf_idx);
  ValueKeyframe *next_kf = current_kf_idx == kfs->size - 1 ? curr_kf : (ValueKeyframe *)list_get(kfs, current_kf_idx + 1);

  return easing_interpolate(curr_kf->easing, timing, curr_kf->timing, next_kf->timing, curr_kf->value, next_kf->value);
}

void value_channel_unload(ValueChannel *channel)
{
  list_free(&channel->keyframes);
}
