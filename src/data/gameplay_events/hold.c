#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "raylib.h"
#include "hold.h"

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
  printf("hold(%d,%d,%.2f)\n", hold->start_timing, hold->end_timing, hold->lane);
}

MeshRenderable hold_load_mesh(Texture2D *texture)
{
  const int VERTICES_COUNT  = 4;
  const int TRIANGLE_COUNT  = 2;
  const int TEXCOORDS_COUNT = 4;
  const int INDICES_COUNT   = TRIANGLE_COUNT * 3;

  MeshRenderable r = { 0 };

  r.mesh.vertexCount    = VERTICES_COUNT;
  r.mesh.triangleCount  = TRIANGLE_COUNT;
  
  r.mesh.vertices  =          (float *)malloc(VERTICES_COUNT * 3 * sizeof(float));
  r.mesh.texcoords =          (float *)malloc(TEXCOORDS_COUNT * 2 * sizeof(float));
  r.mesh.indices   = (unsigned short *)malloc(INDICES_COUNT * sizeof(unsigned short));

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
  unsigned short idx[INDICES_COUNT] = {0, 1, 2, 0, 2, 3};

  for (int i = 0; i < VERTICES_COUNT * 3 ; i++) r.mesh.vertices[i]  = v[i];
  for (int i = 0; i < TEXCOORDS_COUNT * 2; i++) r.mesh.texcoords[i] = uv[i];
  for (int i = 0; i < INDICES_COUNT      ; i++) r.mesh.indices[i]   = idx[i];

  UploadMesh(&r.mesh, false);

  r.material = LoadMaterialDefault();
  r.material.maps[MATERIAL_MAP_DIFFUSE].texture = *texture;

  return r;
}

int hold_compare_start_fp_asc(const void *a, const void *b)
{
  const Hold *hold_a = (const Hold *) a;
  const Hold *hold_b = (const Hold *) b;
  double sfp_a = hold_a->start_fp;
  double sfp_b = hold_b->start_fp;
  if (fabs(sfp_a - sfp_b) > 1e-6)
    return sfp_a < sfp_b ? -1 : 1;
  return 0;
}

void hold_parse_aff(List *holds, const char* line, int *current_tg)
{
  int start_timing, end_timing;
  float lane;
  int matched = sscanf(line, "hold(%d,%d,%f);", &start_timing, &end_timing, &lane);
  if (matched == 3)
  {
    Hold hold = {
      .lane = lane,
      .start_timing = start_timing,
      .end_timing   = end_timing,
      .timing_group = *current_tg,
    };
    list_push(holds, &hold);
  }
}

int hold_compare_id_eq(const void *a, const void *b)
{
  const Hold *hold_a = (const Hold *) a;
  const Hold *hold_b = (const Hold *) b;
  return hold_a->id == hold_b->id ? 0 : -1;
}

void hold_build_tree(ItvTree *tree, List *list, int low, int high)
{
  if (low > high) return;
  int mid = (low + high) / 2;
  Hold *hold = (Hold *)list_get(list, mid);
  Interval i = { .low = &hold->start_fp, .high = &hold->end_fp };
  itv_tree_insert(tree, hold, i);
  hold_build_tree(tree, list, low, mid - 1);
  hold_build_tree(tree, list, mid + 1, high);
}

int hold_render_compare_start_fp_asc(const void *a, const void *b)
{
  const HoldRender *hold_a = (const HoldRender *) a;
  const HoldRender *hold_b = (const HoldRender *) b;
  double sfp_a = hold_a->start_fp;
  double sfp_b = hold_b->start_fp;
  if (fabs(sfp_a - sfp_b) > 1e-6)
    return sfp_a < sfp_b ? -1 : 1;
  return 0;
}
