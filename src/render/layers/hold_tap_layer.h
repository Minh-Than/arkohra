#ifndef HOLD_TAP_LAYER_H
#define HOLD_TAP_LAYER_H

#include "raylib.h"
#include "render/mesh_renderable.h"

typedef struct
{
  MeshRenderable hold, tap, connector;
  RenderTexture2D layer;
} HoldTapRenderer;

#endif // HOLD_TAP_LAYER_H
