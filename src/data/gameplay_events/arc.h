#ifndef ARC_H
#define ARC_H

#include <string.h>
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
  float start_fp, end_fp;
  int color;
  ArcType type;
  bool is_selected, is_void, is_head;
  char sfx[128];
} Arc;

void arc_print(const void *elem);

// ARC SEGMENT
typedef struct {
  Arc *arc;
  MeshRenderable mesh_r, shadow_r;
  float start_fp, end_fp;
} ArcSegment;

void arc_segment_generate_mesh(List *arc_segments_list, Arc *arc, Texture2D *texture, List *timing_events, RenderContext *render_ctx);
void shadow_segment_generate_mesh(List *arc_segments_list, Arc *arc, List *timing_events, RenderContext *render_ctx);
int arc_segment_compare_start_fp_asc(const void *a, const void *b);

#endif // ARC_H
