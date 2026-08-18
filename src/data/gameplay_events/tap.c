#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "tap.h"
#include "raylib.h"
#include "raymath.h"
#include "render/mesh_renderable.h"
#include "gameplay/arc_formula.h"

void tap_print(const void *elem)
{
  const Tap *tap = (const Tap *)elem;
  printf("(%d,%.2f)", tap->timing, tap->lane);
}

void tap_render_test(MeshRenderable *tap_r, Tap *tap, float z_pos, float z_scale)
{
  Matrix tr = MatrixMultiply(
    MatrixRotateX(-180.0f * DEG2RAD),
    MatrixMultiply(MatrixScale(1.0f, 1.0f, z_scale),
                   MatrixTranslate(lane_to_world_x(tap->lane), 0.0f, z_pos)));
  DrawMesh(tap_r->mesh, tap_r->material, tr);
}

MeshRenderable tap_load_mesh(Texture2D *texture)
{
  const int VERTICES_COUNT  = 4;
  const int TRIANGLE_COUNT  = 2;
  const int TEXCOORDS_COUNT = 4;
  const int INDICES_COUNT   = 3;

  MeshRenderable r = { 0 };

  r.mesh.vertexCount    = VERTICES_COUNT;
  r.mesh.triangleCount  = TRIANGLE_COUNT;

  r.mesh.vertices  =          (float *)malloc(VERTICES_COUNT * 3 * sizeof(float));
  r.mesh.texcoords =          (float *)malloc(TEXCOORDS_COUNT * 2 * sizeof(float));
  r.mesh.indices   = (unsigned short *)malloc(INDICES_COUNT * 2 * sizeof(unsigned short));

  float v[VERTICES_COUNT * 3] = {
    -1.08f, 0.0f, 0.0f,
     1.08f, 0.0f, 0.0f,
     1.08f, 0.0f, 1.0f,
    -1.08f, 0.0f, 1.0f,
  };
  float uv[TEXCOORDS_COUNT * 2] = {
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
  };
  unsigned short idx[INDICES_COUNT * 2] = {0, 1, 2, 0, 2, 3};

  for (int i = 0; i < VERTICES_COUNT * 3 ; i++) r.mesh.vertices[i]  = v[i];
  for (int i = 0; i < TEXCOORDS_COUNT * 2; i++) r.mesh.texcoords[i] = uv[i];
  for (int i = 0; i < INDICES_COUNT * 2  ; i++) r.mesh.indices[i]   = idx[i];

  UploadMesh(&r.mesh, false);

  r.material = LoadMaterialDefault();
  r.material.maps[MATERIAL_MAP_DIFFUSE].texture = *texture;

  return r;
}
