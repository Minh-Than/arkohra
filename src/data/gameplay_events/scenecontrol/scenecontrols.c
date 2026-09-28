#include <stdio.h>
#include <string.h>
#include "scenecontrols.h"
#include "data/keyframe/value_channel.h"

SCType scenecontrol_determine_type(char *str_name)
{
  if (strstr(str_name, "hidegroup") != NULL)     return SC_HIDEGROUP;
  if (strstr(str_name, "groupalpha") != NULL)    return SC_GROUPALPHA;
  if (strstr(str_name, "enwidencamera") != NULL) return SC_ENWIDENCAMERA;
  if (strstr(str_name, "enwidenlanes") != NULL)  return SC_ENWIDENLANES;
  return SC_NONE;
}

void scenecontrol_parse_aff(ChartTimingGroup *tg, const char* line)
{
  int timing;
  char name[128];
  float duration, value;
  int matched = sscanf(line, "scenecontrol(%d,%127[^,],%f,%f);", &timing, name, &duration, &value);
  if (matched == 4)
  {
    SCType sc_type = scenecontrol_determine_type(name);
    switch (sc_type)
    {
      case SC_HIDEGROUP:
      {
        ValueKeyframe kf = { .start_timing = timing, .end_timing = timing, .next_value = value, .easing = E_STEP_END };
        list_push(&tg->hidegroup_channel.keyframes, &kf);
        break;
      }
      case SC_GROUPALPHA:
      {
        ValueKeyframe kf = { .start_timing = timing, .end_timing = timing + (int)duration, .next_value = value, .easing = E_LINEAR };
        list_push(&tg->groupalpha_channel.keyframes, &kf);
        break;
      }
      case SC_ENWIDENCAMERA:
      {
        ValueKeyframe kf = { .start_timing = timing, .end_timing = timing + (int)duration, .next_value = value, .easing = E_LINEAR };
        list_push(&tg->enwidencamera_channel.keyframes, &kf);
        break;
      }
      case SC_ENWIDENLANES:
      {
        ValueKeyframe kf = { .start_timing = timing, .end_timing = timing + (int)duration, .next_value = value, .easing = E_LINEAR };
        list_push(&tg->enwidenlanes_channel.keyframes, &kf);
        break;
      }
      default: break;
    }
  }
}
