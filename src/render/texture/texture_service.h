#ifndef TEXTURE_SERVICE_H
#define TEXTURE_SERVICE_H

#include "raylib.h"

typedef struct
{
  Texture2D background;
  Texture2D kohra;
  Texture2D track;
  Texture2D lane_div;
  Texture2D critical_line;
  Texture2D sky_input_line;
  Texture2D sky_label;
  Texture2D single_line;
  Texture2D pause_button;
  Texture2D info_panel;
  Texture2D jacket_bg;
  Texture2D jacket_img;
  Texture2D jacket_diff;

  Texture2D tap, hold, arc, arc_height_indicator, arctap, arctap_shadow;
} TextureGroup;

TextureGroup textures_init();
void textures_unload(TextureGroup* texture_group);

#endif // TEXTURE_SERVICE_H
