#ifndef SKIN_SIDE
#define SKIN_SIDE

#include "raylib.h"
typedef enum
{
  SK_LIGHT,
  SK_CONFLICT,
  SK_COLORLESS,
  SK_BLACK,
  SK_NIJUUSEI,
  SK_REI,
  SK_CONFLICTVS,
  SK_TEMPESTISSIMO,
  SK_FINALE,
  SK_PENTIMENT,
  SK_ARCANA,
} SkinSide;

SkinSide skin_side_get_by_string(char *str);
Texture2D skin_side_get_track(SkinSide side);
Texture2D skin_side_get_hold(SkinSide side);
Texture2D skin_side_get_tap(SkinSide side);
Texture2D skin_side_get_arctap(SkinSide side);
void skin_side_print(SkinSide side);
#endif  // SKIN_SIDE
