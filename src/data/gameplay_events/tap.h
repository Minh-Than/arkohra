#ifndef TAP_H
#define TAP_H

#include "data/custom_types/custom_types.h"
#include "raylib.h"
#include "render/mesh_renderable.h"
#include "render/render_service.h"
typedef struct
{
  List  connector_x, connector_y;
  float lane;
  int   timing, timing_group;
  double fp;
  bool  is_selected;
} Tap;

void tap_print(const void *elem);
void draw_tap(MeshRenderable *tap_r, Tap *tap, RenderContext *render_ctx, float base_bpm, float scroll_speed, double curr_fp);
MeshRenderable tap_load_mesh(Texture2D *texture);

typedef struct {
  Tap   *tap;
  double fp;
} TapFP;

int tapfp_compare_fp_asc(const void *a, const void *b);

#endif // TAP_H
