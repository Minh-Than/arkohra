#ifndef ARC_H
#define ARC_H

#include <string.h>
#include "data/chart_timing_groups/chart_timing_group.h"
#include "raylib.h"
#include "render/mesh_renderable.h"
#include "data/custom_types/custom_types.h"
#include "render/render_service.h"

// ARC
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

typedef struct {
  List arctaps;
  float x1, y1, x2, y2;
  float arc_res;
  int start_timing, end_timing, timing_group;
  double start_fp, end_fp;
  int color;
  ArcType type;
  bool is_selected, is_void, is_head, has_height_indicator;
  char sfx[128];
} Arc;

int arc_compare_start_timing_asc(const void *a, const void *b);
MeshRenderable generate_arccap_mesh(Texture2D *texture, RenderContext *render_ctx);
void arc_print(const void *elem);

// ARC SEGMENT
typedef struct {
  Arc *arc;
  MeshRenderable mesh_r, shadow_r;
  double start_fp, end_fp;
} ArcSegment;

void generate_segments(ChartTimingGroup *tg, Arc *arc, Texture2D *texture, RenderContext *render_ctx);
void generate_arc_body_mesh(List *timing_events, List *arc_segments_list, Arc *arc, Texture2D *texture, RenderContext *render_ctx,
                            float curr_timing, float increment);
MeshRenderable generate_arc_head_mesh(Texture2D *texture, RenderContext *render_ctx);
void generate_arc_shadow_mesh(List *timing_events, List *arc_segments_list, Arc *arc, RenderContext *render_ctx, float curr_timing, float increment, int i);
int arc_segment_compare_start_fp_asc(const void *a, const void *b);
int arc_segment_const_void_compare_start_fp_asc(const void *a, const void *b);
void arc_segment_build_tree(ItvTree *tree, List *list, int low, int high);

void draw_arc_shadow(ChartTimingGroup *tg, ArcSegment *arc_segment, RenderContext *render_ctx, float current_ms, float curr_bpm, float z_pos);
void draw_arc_head(ChartTimingGroup *tg, ArcSegment *arc_segment, RenderContext *render_ctx, MeshRenderable *mesh_r,
                   float current_ms, float curr_bpm, float base_bpm, float scroll_speed, double curr_fp);
void draw_arc_segment(ChartTimingGroup *tg, ArcSegment *arc_segment, RenderContext *render_ctx, float current_ms, float curr_bpm, float z_pos);
void draw_height_indicator(ArcSegment *arc_segment, Mesh *mesh, Material mat, float z_pos);
void draw_arccap(ArcSegment *arc_segment, Mesh *mesh, Material mat, float scale, float alpha, float current_ms);
bool should_draw_height_indicator(ArcSegment *arc_segment);

#endif // ARC_H
