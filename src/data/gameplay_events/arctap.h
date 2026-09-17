#ifndef ARCTAP_H
#define ARCTAP_H

#include "arc.h"
#include "raylib.h"
#include "render/mesh_renderable.h"

typedef struct
{
  Arc   *arc;
  float width;
  int   timing, timing_group;
  double fp;
  bool  is_selected;
} ArcTap;

void draw_arctap(MeshRenderable *arctap_r, ArcTap *arctap, double z_pos);
MeshRenderable arctap_load_mesh(Texture2D *texture);
MeshRenderable arctap_shadow_load_mesh(Texture2D *texture);
void parse_arctaps(const char *line, List *out);

typedef struct {
  ArcTap *arctap;
  double fp;
} ArcTapFP;

void arctap_fp_print(const void *elem);
int arctapfp_compare_fp_asc(const void *a, const void *b);

#endif // ARCTAP_H
