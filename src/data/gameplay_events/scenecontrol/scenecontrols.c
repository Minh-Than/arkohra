#include <string.h>
#include "scenecontrols.h"

SCType scenecontrol_determine_type(char *str_name)
{
  if (strstr(str_name, (const char*)"hidegroup") != NULL)     return SC_HIDEGROUP;
  if (strstr(str_name, (const char*)"groupalpha") != NULL)    return SC_GROUPALPHA;
  if (strstr(str_name, (const char*)"enwidencamera") != NULL) return SC_ENWIDENCAMERA;
  return SC_NONE;
}
