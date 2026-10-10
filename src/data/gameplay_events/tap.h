#ifndef TAP_H
#define TAP_H

#include "data/chart_settings/chart_settings.h"
#include "data/custom_types/dynamic_list.h"
#include "raylib.h"
#include "render/mesh_renderable.h"

typedef struct {
  int id;
  List  connector_x, connector_y;
  float lane;
  int   timing, timing_group;
  double fp;
  bool  is_selected;
} Tap;

void tap_print(const void *elem);
void draw_tap_connection(Tap *tap, ChartSettings *chart_settings, float base_bpm, float scroll_speed, double curr_fp, float add_fade);
MeshRenderable tap_load_mesh(Texture2D *texture);
void tap_parse_aff(List *taps, const char* line, int *current_tg);
int tap_compare_fp_asc(const void *a, const void *b);
int tap_compare_id_eq(const void *a, const void *b);

typedef struct {
  int tap_id, timing_group;
  double fp;
} TapRender;

int tap_render_compare_fp_asc(const void *a, const void *b);

#endif // TAP_H
