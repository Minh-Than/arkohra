#include <stdio.h>
#include <stdlib.h>
#include "value_channel.h"
#include "data/custom_types/dynamic_list.h"
#include "data/keyframe/easings.h"

void value_kf_print(ValueKeyframe kf)
{
  printf("[%d, %d], %.2f -> %.2f\n", kf.start_timing, kf.end_timing, kf.prev_value, kf.next_value);
}

int value_kf_compare_start_timing_asc(const void *a, const void *b)
{
  const ValueKeyframe *kf_a = (const ValueKeyframe *) a;
  const ValueKeyframe *kf_b = (const ValueKeyframe *) b;
  return kf_a->start_timing - kf_b->start_timing;
}

ValueChannel value_channel_init()
{
  List keyframes; list_init(&keyframes, sizeof(ValueKeyframe));
  ValueChannel channel = { .keyframes = keyframes };
  return channel;
}

void value_channel_print(ValueChannel *channel)
{
  for (int i = 0; i < channel->keyframes.size; i++)
  {
    ValueKeyframe *kf = (ValueKeyframe *)list_get(&channel->keyframes, i);
    value_kf_print(*kf);
  }
}

// Keyframes list HAS to be sorted
float value_channel_interpolate(ValueChannel *channel, int timing)
{
  if (channel == NULL) return 0; // Ideally this gatecheck should never happen as this will be called during hot load

  List *kfs = &channel->keyframes;
  if(kfs->data == NULL || kfs->size == 0) return 0;

  ValueKeyframe target_kf = { .start_timing = timing };
  int curr_kf_idx = bisect_left(kfs, &target_kf, value_kf_compare_start_timing_asc) - 1;
  if (curr_kf_idx < 0)                  curr_kf_idx = 0;
  else if (curr_kf_idx > kfs->size - 1) curr_kf_idx = kfs->size - 1;
  ValueKeyframe *curr_kf = (ValueKeyframe *)list_get(kfs, curr_kf_idx);

  return easing_interpolate(curr_kf->easing, timing, curr_kf->start_timing, curr_kf->end_timing, curr_kf->prev_value, curr_kf->next_value);
}

void value_channel_unload(ValueChannel *channel)
{
  list_free(&channel->keyframes);
}
