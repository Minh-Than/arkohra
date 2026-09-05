#include <stdlib.h>
#include "arctap.h"
#include "gameplay/arc_formula.h"
#include "rlgl.h"
#include "raymath.h"

void arctap_render_test(MeshRenderable *arctap_r, ArcTap *arctap, float z_pos)
{
  Matrix tr = MatrixTranslate(
    arc_world_x_at(arctap->timing, arctap->arc),
    arc_world_y_at(arctap->timing, arctap->arc),
    z_pos
  );
  DrawMesh(arctap_r->mesh, arctap_r->material, tr);
}

MeshRenderable arctap_load_mesh(Texture2D *texture)
{
  const int VERTICES_COUNT  = 6 * 4;
  const int TRIANGLE_COUNT  = 6 * 2;
  const int TEXCOORDS_COUNT = 6 * 4;
  const int INDICES_COUNT   = TRIANGLE_COUNT * 3;

  MeshRenderable r = { 0 };

  r.mesh.vertexCount    = VERTICES_COUNT;
  r.mesh.triangleCount  = TRIANGLE_COUNT;
  
  r.mesh.vertices  =          (float *)malloc(VERTICES_COUNT * 3 * sizeof(float));
  r.mesh.normals   =          (float *)malloc(VERTICES_COUNT * 3 * sizeof(float));
  r.mesh.texcoords =          (float *)malloc(TEXCOORDS_COUNT * 2 * sizeof(float));
  r.mesh.indices   = (unsigned short *)malloc(INDICES_COUNT * sizeof(unsigned short));

  float v[VERTICES_COUNT * 3] = {
    -1.19f, -0.2f, 0.0f,   1.19f, -0.2f, 0.0f,   1.19f, -0.8f, 0.0f,  -1.19f, -0.8f, 0.0f,   // front
    -1.19f, -0.2f,-1.2f,   1.19f, -0.2f,-1.2f,   1.19f, -0.8f,-1.2f,  -1.19f, -0.8f,-1.2f,   // back
    -1.19f, -0.2f,-1.2f,  -1.19f, -0.2f, 0.0f,  -1.19f, -0.8f, 0.0f,  -1.19f, -0.8f,-1.2f,   // left
     1.19f, -0.2f,-1.2f,   1.19f, -0.2f, 0.0f,   1.19f, -0.8f, 0.0f,   1.19f, -0.8f,-1.2f,   // right
    -1.19f, -0.8f, 0.0f,   1.19f, -0.8f, 0.0f,   1.19f, -0.8f,-1.2f,  -1.19f, -0.8f,-1.2f,   // bottom
    -1.19f, -0.2f, 0.0f,   1.19f, -0.2f, 0.0f,   1.19f, -0.2f,-1.2f,  -1.19f, -0.2f,-1.2f,   // top
  };
  float n[VERTICES_COUNT * 3] = {
     0, 0, 1,   0, 0, 1,   0, 0, 1,   0, 0, 1,   // front
     0, 0,-1,   0, 0,-1,   0, 0,-1,   0, 0,-1,   // back
    -1, 0, 0,  -1, 0, 0,  -1, 0, 0,  -1, 0, 0,   // left
     1, 0, 0,   1, 0, 0,   1, 0, 0,   1, 0, 0,   // right
     0,-1, 0,   0,-1, 0,   0,-1, 0,   0,-1, 0,   // bottom
     0, 1, 0,   0, 1, 0,   0, 1, 0,   0, 1, 0,   // top
  };
  float uv[TEXCOORDS_COUNT * 2] = {
    0,1,  1,1,  1,0,  0,0,   // front
    0,1,  1,1,  1,0,  0,0,   // back
    0,1,  1,1,  1,0,  0,0,   // left
    0,1,  1,1,  1,0,  0,0,   // right
    0,1,  1,1,  1,0,  0,0,   // bottom
    0,1,  1,1,  1,0,  0,0,   // top
  };
  unsigned short idx[INDICES_COUNT];
  for (int face = 0; face < 6; face++) {
    int base    = face * 4;
    int i       = face * 6;
    idx[i + 0]  = base + 0;
    idx[i + 1]  = base + 1;
    idx[i + 2]  = base + 2;
    idx[i + 3]  = base + 0;
    idx[i + 4]  = base + 2;
    idx[i + 5]  = base + 3;
  }

  for (int i = 0; i < VERTICES_COUNT * 3 ; i++) r.mesh.vertices[i]  = v[i];
  for (int i = 0; i < VERTICES_COUNT * 3 ; i++) r.mesh.normals[i]   = n[i];
  for (int i = 0; i < TEXCOORDS_COUNT * 2; i++) r.mesh.texcoords[i] = uv[i];
  for (int i = 0; i < INDICES_COUNT      ; i++) r.mesh.indices[i]   = idx[i];

  UploadMesh(&r.mesh, false);

  r.material = LoadMaterialDefault();
  r.material.maps[MATERIAL_MAP_DIFFUSE].texture = *texture;

  return r;
}

MeshRenderable arctap_shadow_load_mesh(Texture2D *texture)
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
    -1.19f, 0.0f, 0.0f,
     1.19f, 0.0f, 0.0f,
     1.19f, 0.0f, 1.2f,
    -1.19f, 0.0f, 1.2f,
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
  r.material.maps[MATERIAL_MAP_DIFFUSE].color = (Color){ 90, 90, 90, 60 };
  // r.material.maps[MATERIAL_MAP_DIFFUSE].texture = *texture;
  // r.material.maps->color = Fade(r.material.maps->color, 0.23);

  return r;
}

void arctap_fp_print(const void *elem)
{
  ArcTapFP *arctap_fp = (ArcTapFP *)elem;
  printf("%f %zu", arctap_fp->fp, (unsigned long)arctap_fp->arctap);
}

int arctapfp_compare_fp_asc(const void *a, const void *b)
{
  const ArcTapFP *arctap_a = (const ArcTapFP *) a;
  const ArcTapFP *arctap_b = (const ArcTapFP *) b;
  double fp_a = arctap_a->fp;
  double fp_b = arctap_b->fp;
  if(fabs(fp_a - fp_b) > 1e-6 && fp_a < fp_b) return -1;
  if(fabs(fp_a - fp_b) > 1e-6 && fp_a > fp_b) return 1;
  return 0;
}
