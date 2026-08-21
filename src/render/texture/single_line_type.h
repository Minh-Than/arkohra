#ifndef SINGLE_LINE_TYPE_H
#define SINGLE_LINE_TYPE_H

#include "render/texture/texture_service.h"
typedef enum
{
  SL_LIGHT,
  SL_CONFLICT,
  SL_NEO,
  SL_NONE
} SingleLineType;

SingleLineType single_line_get_by_string(char *str);
void single_line_load(SingleLineType sl_type, TextureGroup *texture_group);
void single_line_print(SingleLineType sl_type);

#endif // SINGLE_LINE_TYPE_H
