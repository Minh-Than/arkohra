#ifndef HOLD_H
#define HOLD_H

#include <stdbool.h>
#include "render/mesh_renderable.h"

typedef struct
{
  double lane;
  int    start_timing, end_timing, timing_group;
  double start_fp, end_fp;
  bool   is_selected, is_active;
} Hold;
void hold_print(const void *elem);
void draw_hold(MeshRenderable *hold_r, Hold *hold, float current_ms, float base_bpm, float scroll_speed, double curr_fp);
MeshRenderable hold_load_mesh(Texture2D *texture);
int hold_const_void_compare_start_fp_asc(const void *a, const void *b);

#endif // HOLD_H
