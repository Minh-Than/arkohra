#include <stdlib.h>
#include <string.h>
#include "gameplay_events.h"

RawEventType determine_type(char *line)
{
  if(line == NULL) return NO_EVENT;

  if (strstr(line, "timinggroup(") != NULL)  return TIMING_GROUP;
  if (strstr(line, "timing(")      != NULL)  return TIMING_EVENT;
  if (strstr(line, "scenecontrol(") != NULL) return SCENECONTROL;

  char c = line[0];
  if (c == '(') return TAP;
  if (c == 'h') return HOLD;
  if (c == 'a') return ARC;
  return NO_EVENT;
}
