#ifndef HOLD_H
#define HOLD_H

#include <stdbool.h>
#include "data/custom_types/custom_types.h"
#include "render/mesh_renderable.h"

typedef struct {
  int id;
  double lane;
  int    start_timing, end_timing, timing_group;
  double start_fp, end_fp;
  bool   is_selected, is_active;
} Hold;

void hold_print(const void *elem);
MeshRenderable hold_load_mesh(Texture2D *texture);
void hold_parse_aff(List *holds, const char* line, int *current_tg);
int hold_compare_start_fp_asc(const void *a, const void *b);
int hold_compare_id_eq(const void *a, const void *b);
void hold_build_tree(ItvTree *tree, List *list, int low, int high);

typedef struct {
  int hold_id, timing_group;
  double start_fp, end_fp;
} HoldRender;
int hold_render_compare_start_fp_asc(const void *a, const void *b);

#endif // HOLD_H
