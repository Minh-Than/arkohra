#ifndef ARCTAP_LAYER_H
#define ARCTAP_LAYER_H

#include "raylib.h"
#include "render/mesh_renderable.h"

typedef struct
{
  MeshRenderable arctap;
  RenderTexture2D layer;
} ArctapRenderer;

#endif // ARCTAP_LAYER_H
