#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "raylib.h"
#include "raymath.h"
#include "hold.h"
#include "gameplay/arc_formula.h"

static void hold_set_active(MeshRenderable *r, bool is_active)
{
  float uv[4] = { 0.0f, 0.5f, 0.5f, 0.0f, };
  if (is_active) for (int i = 0; i < 4; i++) { uv[i] += 0.5f; }
  for (int i = 0, counter = 0; i < 4 * 2; i += 2, counter++) r->mesh.texcoords[i] = uv[i - counter];
  UpdateMeshBuffer(r->mesh, 1, r->mesh.texcoords, 4 * 2 * sizeof(float), 0);
}

void hold_print(const void *elem)
{
  const Hold *hold = (const Hold *)elem;
  printf("hold(%d,%d,%.2f)", hold->start_timing, hold->end_timing, hold->lane);
}

void hold_render_test(MeshRenderable *hold_r, Hold *hold, float z_pos, float z_scale, float alpha)
{
  hold_r->material.maps->color = ColorAlpha(hold_r->material.maps->color, alpha);
  Matrix tr = MatrixMultiply(
    MatrixRotateX(-180.0f * DEG2RAD),
    MatrixMultiply(MatrixScale(1.0f, 1.0f, -z_scale),
                   MatrixTranslate(lane_to_world_x(hold->lane), 0.0f, z_pos)));
  BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
    DrawMesh(hold_r->mesh, hold_r->material, tr);
  EndBlendMode();
}

MeshRenderable hold_load_mesh(Texture2D *texture)
{
  const int VERTICES_COUNT  = 4;
  const int TRIANGLE_COUNT  = 4;
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
    0.5f, 1.0f,
    0.5f, 0.0f,
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
