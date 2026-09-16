#include "texture_service.h"
#include "raylib.h"

TextureGroup textures_init()
{
  Texture2D kohracube_texture = LoadTexture("resources/gameplay/kohar.png");
  GenTextureMipmaps(&kohracube_texture);
  SetTextureFilter(kohracube_texture, TEXTURE_FILTER_TRILINEAR);

  return (TextureGroup){ .kohra = kohracube_texture };
}

void textures_unload(TextureGroup* texture_group)
{
  UnloadTexture(texture_group->kohra);
}
