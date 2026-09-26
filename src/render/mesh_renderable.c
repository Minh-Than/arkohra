#include <stdlib.h>
#include <string.h>
#include "mesh_renderable.h"
#include "raylib.h"

void renderable_set_transforms(MeshRenderable *renderable, Matrix *transforms, int count)
{
  renderable->transforms = (Matrix *)malloc(count * sizeof(Matrix));
  renderable->transform_count = count;
  memcpy(renderable->transforms, transforms, count * sizeof(Matrix));
}

void renderable_draw(MeshRenderable *r)
{
  for(int i = 0; i < r->transform_count; i++)
    DrawMesh(r->mesh, r->material, r->transforms[i]);
}

void renderable_unload(MeshRenderable *renderable)
{
    if (renderable->initial_texcoords) free(renderable->initial_texcoords);
    free(renderable->transforms);
    UnloadMesh(renderable->mesh);

    renderable->material.maps[MATERIAL_MAP_DIFFUSE].texture = (Texture2D){ 0 };
    renderable->material.shader = (Shader){ 0 };
    UnloadMaterial(renderable->material);
}
