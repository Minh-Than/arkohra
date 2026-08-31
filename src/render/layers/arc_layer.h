#ifndef ARC_LAYER_H
#define ARC_LAYER_H

#include "raylib.h"
#include "render/mesh_renderable.h"

typedef struct
{
  MeshRenderable height_indicator, arctap_shadow;
  RenderTexture2D layer;
} ArcRenderer;

#endif // ARC_LAYER_H
