#include <stdlib.h>
#include "value_channel.h"

void value_channel_init(ValueChannel *channel)
{
  list_init(&channel->keyframes, sizeof(ValueKeyframe));
}
