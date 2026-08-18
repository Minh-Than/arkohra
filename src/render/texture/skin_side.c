#include <stdio.h>
#include "skin_side.h"
#include "raylib.h"

SkinSide skin_side_get_by_string(char *str)
{
  if (TextIsEqual(str, "light"))        return LIGHT;
  if (TextIsEqual(str, "conflict"))     return CONFLICT;
  if (TextIsEqual(str, "colorless"))    return COLORLESS;
  if (TextIsEqual(str, "black"))        return SK_BLACK;
  if (TextIsEqual(str, "nijuusei"))     return NIJUUSEI;
  if (TextIsEqual(str, "rei"))          return REI;
  if (TextIsEqual(str, "conflictvs"))   return CONFLICTVS;
  if (TextIsEqual(str, "tempestissimo"))return TEMPESTISSIMO;
  if (TextIsEqual(str, "finale"))       return FINALE;
  if (TextIsEqual(str, "pentiment"))    return PENTIMENT;
  if (TextIsEqual(str, "arcana"))       return ARCANA;
  return LIGHT;
}

void skin_side_load_track(SkinSide side, TextureGroup *texture_group)
{
  switch (side)
  {
    default:
    case LIGHT:         texture_group->track = LoadTexture("resources/gameplay/Track/TrackWhite.png"); break;
    case CONFLICT:      texture_group->track = LoadTexture("resources/gameplay/Track/TrackConflict.png"); break;
    case COLORLESS:     texture_group->track = LoadTexture("resources/gameplay/Track/TrackColorless.png"); break;
    case SK_BLACK:      texture_group->track = LoadTexture("resources/gameplay/Track/TrackBlack.png"); break;
    case NIJUUSEI:      texture_group->track = LoadTexture("resources/gameplay/Track/TrackNijuusei.png"); break;
    case REI:           texture_group->track = LoadTexture("resources/gameplay/Track/TrackRei.png"); break;
    case CONFLICTVS:    texture_group->track = LoadTexture("resources/gameplay/Track/TrackConflictVs.png"); break;
    case TEMPESTISSIMO: texture_group->track = LoadTexture("resources/gameplay/Track/TrackTempestissimo.png"); break;
    case FINALE:        texture_group->track = LoadTexture("resources/gameplay/Track/TrackFinale.png"); break;
    case PENTIMENT:     texture_group->track = LoadTexture("resources/gameplay/Track/TrackPentiment.png"); break;
    case ARCANA:        texture_group->track = LoadTexture("resources/gameplay/Track/TrackArcana.png"); break;
  }
}

void skin_side_load_hold(SkinSide side, TextureGroup *texture_group)
{
  switch (side)
  {
    default:
    case LIGHT:     texture_group->hold = LoadTexture("resources/gameplay/Note/Light/HoldNoteLight.png"); break;
    case CONFLICT:  texture_group->hold = LoadTexture("resources/gameplay/Note/Conflict/HoldNoteConflict.png"); break;
  }
}

void skin_side_load_tap(SkinSide side, TextureGroup *texture_group)
{
  switch (side)
  {
    default:
    case LIGHT:     texture_group->tap = LoadTexture("resources/gameplay/Note/Light/TapNoteLight.png"); break;
    case CONFLICT:  texture_group->tap = LoadTexture("resources/gameplay/Note/Conflict/TapNoteConflict.png"); break;
  }
}

void skin_side_load_arctap(SkinSide side, TextureGroup *texture_group)
{
  switch (side)
  {
    default:
    case LIGHT:     texture_group->arctap = LoadTexture("resources/gameplay/Note/Light/ArcTapLight.png"); break;
    case CONFLICT:  texture_group->arctap = LoadTexture("resources/gameplay/Note/Conflict/ArcTapConflict.png"); break;
  }
}

void skin_side_print(SkinSide side)
{
  char str[16];
  switch (side)
  {
    case LIGHT:         TextCopy(str, "Light");
    case CONFLICT:      TextCopy(str, "Conflict");
    case COLORLESS:     TextCopy(str, "Colorless");
    case SK_BLACK:      TextCopy(str, "Black");
    case NIJUUSEI:      TextCopy(str, "Nijuusei");
    case REI:           TextCopy(str, "Rei");
    case CONFLICTVS:    TextCopy(str, "ConflictVs");
    case TEMPESTISSIMO: TextCopy(str, "Tempestissimo");
    case FINALE:        TextCopy(str, "Finale");
    case PENTIMENT:     TextCopy(str, "Pentinent");
    case ARCANA:        TextCopy(str, "Arcana");
  }

  printf("Chart side: %s\n", str);
}
