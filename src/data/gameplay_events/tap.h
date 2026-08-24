#ifndef TAP_H
#define TAP_H

#include "data/custom_types/custom_types.h"
#include "raylib.h"
#include "render/mesh_renderable.h"
typedef struct
{
  List  connector_x, connector_y;
  float lane;
  int   timing, timing_group;
  float fp;
  bool  is_selected;
} Tap;

void tap_print(const void *elem);
void tap_render_test(MeshRenderable *tap_r, Tap *tap, float fp, float z_scale);
MeshRenderable tap_load_mesh(Texture2D *texture);

typedef struct {
  Tap   *tap;
  float fp;
} TapFP;

int tapfp_compare_fp_asc(const void *a, const void *b);

#endif // TAP_H
