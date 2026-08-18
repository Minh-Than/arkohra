#ifndef SKIN_SIDE
#define SKIN_SIDE

#include "render/texture/texture_service.h"
typedef enum
{
  LIGHT,
  CONFLICT,
  COLORLESS,
  SK_BLACK,
  NIJUUSEI,
  REI,
  CONFLICTVS,
  TEMPESTISSIMO,
  FINALE,
  PENTIMENT,
  ARCANA,
} SkinSide;

SkinSide skin_side_get_by_string(char *str);
void skin_side_load_track(SkinSide side, TextureGroup *texture_group);
void skin_side_load_hold(SkinSide side, TextureGroup *texture_group);
void skin_side_load_tap(SkinSide side, TextureGroup *texture_group);
void skin_side_load_arctap(SkinSide side, TextureGroup *texture_group);
void skin_side_print(SkinSide side);
#endif  // SKIN_SIDE
