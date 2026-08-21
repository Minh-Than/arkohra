#include <stdio.h>
#include "single_line_type.h"
#include "raylib.h"

SingleLineType single_line_get_by_string(char *str)
{
  if (TextIsEqual(str, "light"))    return SL_LIGHT;
  if (TextIsEqual(str, "conflict")) return SL_CONFLICT;
  if (TextIsEqual(str, "neo"))      return SL_NEO;
  return SL_NONE;
}

void single_line_load(SingleLineType sl_type, TextureGroup *texture_group)
{
  switch (sl_type)
  {
    case SL_LIGHT:      texture_group->single_line = LoadTexture("resources/gameplay/SingleLine/SingleLineLight.png"); break;
    case SL_CONFLICT:   texture_group->single_line = LoadTexture("resources/gameplay/SingleLine/SingleLineConflict.png"); break;
    case SL_NEO:        texture_group->single_line = LoadTexture("resources/gameplay/SingleLine/SingleLineNeo.png"); break;
    default:            texture_group->single_line = LoadTexture("resources/gameplay/SingleLine/SingleLineNone.png"); break;
  }
}

void single_line_print(SingleLineType sl_type)
{
  char str[16];
  switch (sl_type)
  {
    case SL_LIGHT:      TextCopy(str, "Light"); break;
    case SL_CONFLICT:   TextCopy(str, "Conflict"); break;
    case SL_NEO:        TextCopy(str, "Neo"); break;
    default:            TextCopy(str, "None"); break;
  }

  printf("Single line: %s\n", str);
}
