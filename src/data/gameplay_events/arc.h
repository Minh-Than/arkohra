#ifndef ARC_H
#define ARC_H

#include <string.h>
#include "data/gameplay_events/arc_clip_shader.h"
#include "raylib.h"
#include "render/mesh_renderable.h"
#include "data/custom_types/custom_types.h"
#include "render/render_service.h"

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
  MeshRenderable mesh_r, shadow_r;
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
void arc_render_test(Arc *arc, MeshRenderable *arctap_shadow, ArcClipShader *arc_clip_shader, float z_pos);
int arc_compare_start_timing_asc(const void *a, const void *b);
MeshRenderable arc_generate_mesh(Arc *arc, Texture2D *texture, List *timing_events, RenderContext *render_ctx);
MeshRenderable shadow_generate_mesh(Arc *arc, List *timing_events, RenderContext *render_ctx);

#endif // ARC_H
