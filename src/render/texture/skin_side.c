#include <stdio.h>
#include "skin_side.h"
#include "raylib.h"

SkinSide skin_side_get_by_string(char *str)
{
  if (TextIsEqual(str, "light"))        return SK_LIGHT;
  if (TextIsEqual(str, "conflict"))     return SK_CONFLICT;
  if (TextIsEqual(str, "colorless"))    return SK_COLORLESS;
  if (TextIsEqual(str, "black"))        return SK_BLACK;
  if (TextIsEqual(str, "nijuusei"))     return SK_NIJUUSEI;
  if (TextIsEqual(str, "rei"))          return SK_REI;
  if (TextIsEqual(str, "conflictvs"))   return SK_CONFLICTVS;
  if (TextIsEqual(str, "tempestissimo"))return SK_TEMPESTISSIMO;
  if (TextIsEqual(str, "finale"))       return SK_FINALE;
  if (TextIsEqual(str, "pentiment"))    return SK_PENTIMENT;
  if (TextIsEqual(str, "arcana"))       return SK_ARCANA;
  return SK_LIGHT;
}

void skin_side_load_track(SkinSide side, TextureGroup *texture_group)
{
  switch (side)
  {
    case SK_LIGHT:         texture_group->track = LoadTexture("resources/gameplay/Track/TrackWhite.png"); break;
    case SK_CONFLICT:      texture_group->track = LoadTexture("resources/gameplay/Track/TrackConflict.png"); break;
    case SK_COLORLESS:     texture_group->track = LoadTexture("resources/gameplay/Track/TrackColorless.png"); break;
    case SK_BLACK:         texture_group->track = LoadTexture("resources/gameplay/Track/TrackBlack.png"); break;
    case SK_NIJUUSEI:      texture_group->track = LoadTexture("resources/gameplay/Track/TrackNijuusei.png"); break;
    case SK_REI:           texture_group->track = LoadTexture("resources/gameplay/Track/TrackRei.png"); break;
    case SK_CONFLICTVS:    texture_group->track = LoadTexture("resources/gameplay/Track/TrackConflictVs.png"); break;
    case SK_TEMPESTISSIMO: texture_group->track = LoadTexture("resources/gameplay/Track/TrackTempestissimo.png"); break;
    case SK_FINALE:        texture_group->track = LoadTexture("resources/gameplay/Track/TrackFinale.png"); break;
    case SK_PENTIMENT:     texture_group->track = LoadTexture("resources/gameplay/Track/TrackPentiment.png"); break;
    case SK_ARCANA:        texture_group->track = LoadTexture("resources/gameplay/Track/TrackArcana.png"); break;
  }
}

void skin_side_load_hold(SkinSide side, TextureGroup *texture_group)
{
  switch (side)
  {
    case SK_LIGHT:    texture_group->hold = LoadTexture("resources/gameplay/Note/Light/HoldNoteLight.png"); break;
    case SK_CONFLICT: texture_group->hold = LoadTexture("resources/gameplay/Note/Conflict/HoldNoteConflict.png"); break;
    default:    texture_group->hold = LoadTexture("resources/gameplay/Note/Light/HoldNoteLight.png"); break;
  }
}

void skin_side_load_tap(SkinSide side, TextureGroup *texture_group)
{
  switch (side)
  {
    case SK_LIGHT:    texture_group->tap = LoadTexture("resources/gameplay/Note/Light/TapNoteLight.png"); break;
    case SK_CONFLICT: texture_group->tap = LoadTexture("resources/gameplay/Note/Conflict/TapNoteConflict.png"); break;
    default: texture_group->tap = LoadTexture("resources/gameplay/Note/Light/TapNoteLight.png"); break;
  }
}

void skin_side_load_arctap(SkinSide side, TextureGroup *texture_group)
{
  switch (side)
  {
    case SK_LIGHT:    texture_group->arctap = LoadTexture("resources/gameplay/Note/Light/ArcTapLight.png"); break;
    case SK_CONFLICT: texture_group->arctap = LoadTexture("resources/gameplay/Note/Conflict/ArcTapConflict.png"); break;
    default: texture_group->arctap = LoadTexture("resources/gameplay/Note/Light/ArcTapLight.png"); break;
  }
}

void skin_side_print(SkinSide side)
{
  char str[16];
  switch (side)
  {
    case SK_LIGHT:         TextCopy(str, "Light"); break;
    case SK_CONFLICT:      TextCopy(str, "Conflict"); break;
    case SK_COLORLESS:     TextCopy(str, "Colorless"); break;
    case SK_BLACK:         TextCopy(str, "Black"); break;
    case SK_NIJUUSEI:      TextCopy(str, "Nijuusei"); break;
    case SK_REI:           TextCopy(str, "Rei"); break;
    case SK_CONFLICTVS:    TextCopy(str, "ConflictVs"); break;
    case SK_TEMPESTISSIMO: TextCopy(str, "Tempestissimo"); break;
    case SK_FINALE:        TextCopy(str, "Finale"); break;
    case SK_PENTIMENT:     TextCopy(str, "Pentinent"); break;
    case SK_ARCANA:        TextCopy(str, "Arcana"); break;
  }

  printf("Chart side: %s\n", str);
}
