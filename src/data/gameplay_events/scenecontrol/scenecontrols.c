#include <string.h>
#include "scenecontrols.h"

SCType scenecontrol_determine_type(char *str_name)
{
  if (strstr(str_name, (const char*)"hidegroup") != NULL)  return SC_HIDEGROUP;
  return SC_NONE;
}
