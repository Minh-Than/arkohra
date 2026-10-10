#ifndef ARCTAP_H
#define ARCTAP_H

#include "raylib.h"
#include "data/custom_types/dynamic_list.h"
#include "render/mesh_renderable.h"
#include "render/texture/skin_side.h"

typedef struct
{
  int id, arc_id;
  float width;
  int timing, timing_group;
  double fp;
  int arc_start_timing;
  bool  is_selected;
} ArcTap;

MeshRenderable arctap_load_mesh(Texture2D *texture);
MeshRenderable arctap_shadow_load_mesh(Texture2D *texture);
void parse_arctaps(const char *line, List *out);
int arctap_compare_fp_asc(const void *a, const void *b);
int arctap_compare_id_eq(const void *a, const void *b);
Color arctap_get_shadow_color(SkinSide side);
float arctap_get_shadow_alpha(SkinSide side);

typedef struct {
  int arctap_id;
  int timing_group;
  double fp;
} ArcTapRender;

int arctap_render_compare_fp_asc(const void *a, const void *b);
int arctap_render_compare_id_eq(const void *a, const void *b);

#endif // ARCTAP_H
