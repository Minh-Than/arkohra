#include <stdlib.h>
#include <string.h>
#include "gameplay_events.h"

RawEventType determine_type(char *line)
{
  if(line == NULL) return NO_EVENT;

  if (strstr(line, (const char*)"timinggroup(") != NULL)  return TIMING_GROUP;
  if (strstr(line, (const char*)"timing(")      != NULL)  return TIMING_EVENT;
  if (strstr(line, (const char*)"scenecontrol(") != NULL) return SCENECONTROL;

  char c = line[0];
  if (c == '(') return TAP;
  if (c == 'h') return HOLD;
  if (c == 'a') return ARC;
  return NO_EVENT;
}
