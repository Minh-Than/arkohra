#include <string.h>
#include "scenecontrols.h"

SCType scenecontrol_determine_type(char *str_name)
{
  if (strstr(str_name, (const char*)"hidegroup") != NULL)  return SC_HIDEGROUP;
  if (strstr(str_name, (const char*)"groupalpha") != NULL) return SC_GROUPALPHA;
  return SC_NONE;
}
