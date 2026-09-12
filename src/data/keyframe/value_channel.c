#include <stdlib.h>
#include "keyframe.h"

void value_channel_init(ValueChannel *channel)
{
  list_init(&channel->keyframes, sizeof(ValueKeyframe));
}
