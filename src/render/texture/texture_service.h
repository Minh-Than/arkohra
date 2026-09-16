#ifndef TEXTURE_SERVICE_H
#define TEXTURE_SERVICE_H

#include "raylib.h"

typedef struct
{
  Texture2D kohra;
} TextureGroup;

TextureGroup textures_init();
void textures_unload(TextureGroup* texture_group);

#endif // TEXTURE_SERVICE_H
