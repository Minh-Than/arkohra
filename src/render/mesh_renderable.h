#ifndef MESH_RENDERABLE_H
#define MESH_RENDERABLE_H

#include "raylib.h"

typedef struct
{
  Mesh mesh;
  Material material;
  Matrix *transforms;
  int transform_count;

  float *initial_texcoords;
} MeshRenderable;

void renderable_set_transforms(MeshRenderable *renderable, Matrix *transforms, int count);
void renderable_draw(MeshRenderable *r);
void renderable_unload(MeshRenderable *r);

#endif // MESH_RENDERABLE_H
