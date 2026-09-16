#ifndef SINGLE_LINE_TYPE_H
#define SINGLE_LINE_TYPE_H

#include "raylib.h"
typedef enum
{
  SL_LIGHT,
  SL_CONFLICT,
  SL_NEO,
  SL_NONE
} SingleLineType;

SingleLineType single_line_get_by_string(char *str);
Texture2D single_line_get(SingleLineType sl_type);
void single_line_print(SingleLineType sl_type);

#endif // SINGLE_LINE_TYPE_H
