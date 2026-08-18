#ifndef SHADOW_LAYER_H
#define SHADOW_LAYER_H

#include "raylib.h"
#include "render/mesh_renderable.h"
typedef struct
{
  MeshRenderable arctap_shadow;
  RenderTexture2D layer;
} ShadowRenderer;

#endif // SHADOW_LAYER_H
