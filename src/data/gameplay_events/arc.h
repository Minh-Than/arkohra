#ifndef ARC_H
#define ARC_H

#include <string.h>
#include <stdbool.h>
#include "data/chart_timing_groups/chart_timing_group.h"
#include "data/custom_types/custom_types.h"
#include "data/gameplay_events/arc_shader.h"
#include "render/mesh_renderable.h"

typedef enum
{
  B = 1,
  S = 2,
  SI = 3,
  SO = 4,
  SISI = 5,
  SOSO = 6,
  SISO = 7,
  SOSI = 8
} ArcType;

ArcType arctype_get_by_string(char *str);

// ARC
struct Arc {
  int id;
  float x1, y1, x2, y2;
  float arc_res;
  int start_timing, end_timing, timing_group;
  double start_fp, end_fp;
  int color;
  ArcType type;
  bool is_selected, is_void, is_head, has_height_indicator;
  char sfx[128];
  struct Arc *prev_arc, *next_arc;
};

MeshRenderable arc_generate_shadow_mesh(Shader *shader);
MeshRenderable arc_generate_head_mesh(Texture2D *texture, Shader *shader);
MeshRenderable arc_generate_arccap_mesh(Texture2D *texture);
MeshRenderable arc_generate_arc_height_mesh(Texture2D *texture);
int arc_compare_start_timing_asc(const void *a, const void *b);
int arc_compare_id_eq(const void *a, const void *b);
Color arc_get_color_low(int color, bool is_color_blind);
Color arc_get_color_high(int color, bool is_color_blind);
struct Arc *arc_get_firstmost_arc(struct Arc* arc);
void arc_parse_aff(List *arcs, List *arctaps, const char* line, int *current_tg, int *arc_counter);
void arc_print(const void *elem);

// ARC SEGMENT RENDERER
typedef struct {
  int arc_id;
  int timing_group;
  double start_fp, end_fp;
  float start_timing, end_timing;
  float arc_start_timing, arc_end_timing;

  float world_x1, world_y1, world_x2, world_y2;
  double z_delta;
} ArcSegmentRender;

MeshRenderable arc_segment_generate_mesh(Texture2D *texture, Shader *shader);
void draw_arc_shadow(ChartTimingGroup *tg, struct Arc *arc, ArcShader *arc_shader, MeshRenderable *mesh_r,
                     float current_ms, float curr_bpm, double z_pos, float add_fade, Matrix tr);
void draw_arc_head(ChartTimingGroup *tg, struct Arc *arc, ArcShader *arc_shader, MeshRenderable *mesh_r,
                   float current_ms, float curr_bpm, float base_bpm, float scroll_speed, double curr_fp, float add_fade, bool colorblind);
void draw_arc_segment(ChartTimingGroup *tg, struct Arc *arc, ArcShader *arc_shader, MeshRenderable *mesh_r,
                      float current_ms, float curr_bpm, double z_pos, float add_fade, bool colorblind, Matrix tr);
void draw_height_indicator(ArcSegmentRender *segment_render, struct Arc *arc, Mesh *mesh, Material mat, double z_pos, float add_fade, bool colorblind);
void draw_arccap(ArcSegmentRender *segment_render, struct Arc *arc, Mesh *mesh, Material mat, float scale, float alpha, float current_ms);
bool should_draw_height_indicator(ArcSegmentRender *segment_render, struct Arc *arc);
int arc_segment_render_compare_start_fp_asc(const void *a, const void *b);
void arc_segment_render_build_tree(ItvTree *tree, List *list, int low, int high);

#endif // ARC_H
