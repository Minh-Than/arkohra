#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include "arc.h"
#include "constants.h"
#include "color_services.h"
#include "resource_util.h"
#include "data/custom_types/custom_types.h"
#include "data/gameplay_events/arctap.h"
#include "gameplay/arc_formula.h"

// ARC
ArcType arctype_get_by_string(char *str)
{
  if (TextIsEqual(str, "b"))    return B;
  if (TextIsEqual(str, "s"))    return S;
  if (TextIsEqual(str, "si"))   return SI;
  if (TextIsEqual(str, "so"))   return SO;
  if (TextIsEqual(str, "sisi")) return SISI;
  if (TextIsEqual(str, "soso")) return SOSO;
  if (TextIsEqual(str, "siso")) return SISO;
  if (TextIsEqual(str, "sosi")) return SOSI;
  return S;
}

int arc_compare_start_timing_asc(const void *a, const void *b)
{
  const struct Arc *arc_a = (const struct Arc *) a;
  const struct Arc *arc_b = (const struct Arc *) b;
  return arc_a->start_timing - arc_b->start_timing;
}

int arc_compare_id_eq(const void *a, const void *b)
{
  const struct Arc *arc_a = (const struct Arc *) a;
  const struct Arc *arc_b = (const struct Arc *) b;
  return arc_a->id == arc_b->id ? 0 : -1;
}

MeshRenderable arc_generate_arccap_mesh(Texture2D *texture)
{
  float half_size_x = 0.02f;
  float half_size_y = 0.035f;

  MeshRenderable r = { 0 };

  const int VERTICES_COUNT  = 4;
  const int TRIANGLE_COUNT  = 2;
  const int TEXCOORDS_COUNT = 4;
  const int INDICES_COUNT   = 3;

  r.mesh.vertexCount    = VERTICES_COUNT;
  r.mesh.triangleCount  = TRIANGLE_COUNT;

  r.mesh.vertices  =          (float *)malloc(VERTICES_COUNT * 3 * sizeof(float));
  r.mesh.texcoords =          (float *)malloc(TEXCOORDS_COUNT * 2 * sizeof(float));
  r.mesh.indices   = (unsigned short *)malloc(INDICES_COUNT * 2 * sizeof(unsigned short));

  float v[VERTICES_COUNT * 3] = {
    -half_size_x, -half_size_y, 0.0f,
     half_size_x, -half_size_y, 0.0f,
     half_size_x,  half_size_y, 0.0f,
    -half_size_x,  half_size_y, 0.0f,
  };
  float uv[TEXCOORDS_COUNT * 2] = {
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
  };
  unsigned short idx[INDICES_COUNT * 2] = {0, 1, 2, 0, 2, 3};

  for (int i = 0; i < VERTICES_COUNT * 3 ; i++) r.mesh.vertices[i]  = v[i];
  for (int i = 0; i < TEXCOORDS_COUNT * 2; i++) r.mesh.texcoords[i] = uv[i];
  for (int i = 0; i < INDICES_COUNT * 2  ; i++) r.mesh.indices[i]   = idx[i];

  UploadMesh(&r.mesh, false);

  r.material = LoadMaterialDefault();
  r.material.maps[MATERIAL_MAP_DIFFUSE].texture = *texture;

  return r;
}

MeshRenderable arc_generate_arc_height_mesh(Texture2D *texture)
{
  Mesh height_indicator_mesh = GenMeshPlane(0.5f, 1, 1, 1);
  UploadMesh(&height_indicator_mesh, false);
  Material height_indicator_material = LoadMaterialDefault();
  height_indicator_material.maps[MATERIAL_MAP_DIFFUSE].texture = *texture;

  MeshRenderable r = { .mesh = height_indicator_mesh, .material = height_indicator_material };
  renderable_set_transforms(&r, (Matrix[]){MatrixIdentity()}, 1);

  return r;
}

Color arc_get_color_low(int color, bool is_color_blind)
{
  if (color == 0)       return is_color_blind ? color_from_rgba(ARC_BLUE_BLIND_LOW_CL) : color_from_rgba(ARC_BLUE_LOW_CL);
  else if (color == 1)  return is_color_blind ? color_from_rgba(ARC_PINK_BLIND_LOW_CL) : color_from_rgba(ARC_PINK_LOW_CL);
  else                  return color_from_rgba(ARC_GREEN_LOW_CL);
}

Color arc_get_color_high(int color, bool is_color_blind)
{
  if (color == 0)       return is_color_blind ? color_from_rgba(ARC_BLUE_BLIND_HIGH_CL) : color_from_rgba(ARC_BLUE_HIGH_CL);
  else if (color == 1)  return is_color_blind ? color_from_rgba(ARC_PINK_BLIND_HIGH_CL) : color_from_rgba(ARC_PINK_HIGH_CL);
  else                  return color_from_rgba(ARC_GREEN_HIGH_CL);
}

struct Arc *arc_get_firstmost_arc(struct Arc* arc)
{
  while (arc->prev_arc) arc = arc->prev_arc;
  return arc;
}

void arc_parse_aff(List *arcs, List *arctaps, const char* line, int *current_tg, int *arc_counter)
{
  char *fields[10];
  int n = 0;
  char copy[256];
  text_copy_bounded(copy, sizeof(copy), line);
  char *p = copy + 4;
  fields[n++] = p;
  while ((p = strchr(p, ',')) != NULL && n < 10)
  {
    *p = '\0';
    fields[n++] = ++p;
  }

  if (n == 10)
  {
    struct Arc arc = {
      .x1 = (float)atof(fields[2]), .y1 = (float)atof(fields[5]),
      .x2 = (float)atof(fields[3]), .y2 = (float)atof(fields[6]),
      .arc_res      = 1.0f,
      .start_timing = atoi(fields[0]),
      .end_timing   = atoi(fields[1]),
      .timing_group = *current_tg,
      .color        = atoi(fields[7]),
      .type         = arctype_get_by_string(fields[4]),
      .is_void      = strncmp(fields[9], "true", 4) == 0 ? true : false,
      .is_head      = true,
      .prev_arc     = NULL,
      .next_arc     = NULL,
      .id = (*arc_counter)++
    };
    text_copy_bounded(arc.sfx, sizeof(arc.sfx), fields[8]);

    List arctap_timings; list_init(&arctap_timings, sizeof(int));
    parse_arctaps(line, &arctap_timings);
    for (int i = 0; i < arctap_timings.size; i++)
    {
      int *timing = (int *)list_get(&arctap_timings, i);
      ArcTap arctap = { 
        .arc_id = arc.id,
        .width = 1.0f,
        .timing = *timing,
        .timing_group = arc.timing_group,
        .arc_start_timing = arc.start_timing
      };
      list_push(arctaps, &arctap);
    }
    list_free(&arctap_timings);

    list_push(arcs, &arc);
  }
}

void arc_print(const void *elem)
{
  const struct Arc *arc = (const struct Arc *)elem;
  printf("arc(%d,%d,%.2f,%.2f,%d,%.2f,%.2f,%d,%s,%d)",
         arc->start_timing, arc->end_timing,
         arc->x1, arc->x2, arc->type, arc->y1, arc->y2,
         arc->color, arc->sfx, arc->is_void);
  printf(";\n");
}

MeshRenderable arc_segment_generate_mesh(Texture2D *texture, Shader *shader)
{
  //   2 6
  //  /| |\
  // 3 | | 7
  // | 1 5 |
  // |/   \|
  // 0     4

  const int VERTICES_COUNT  = 8;
  const int TRIANGLE_COUNT  = 4;
  const int TEXCOORDS_COUNT = 8;
  const int INDICES_COUNT   = TRIANGLE_COUNT * 3;

  MeshRenderable r = { 0 };

  r.mesh.vertexCount    = VERTICES_COUNT;
  r.mesh.triangleCount  = TRIANGLE_COUNT;
  
  r.mesh.vertices  =          (float *)malloc(VERTICES_COUNT  * 3 * sizeof(float));
  r.mesh.normals   =          (float *)malloc(VERTICES_COUNT  * 3 * sizeof(float));
  r.mesh.texcoords =          (float *)malloc(TEXCOORDS_COUNT * 2 * sizeof(float));
  r.mesh.indices   = (unsigned short *)malloc(INDICES_COUNT * sizeof(unsigned short));

  float v[VERTICES_COUNT * 3] = {
    -0.0433f, -0.025f, 0.0f,   0.0f, 0.05f, 0.0f,   0.0f, 0.05f, 1.0f,  -0.0433f, -0.025f, 1.0f,  // 0, 1, 2, 3
     0.0433f, -0.025f, 0.0f,   0.0f, 0.05f, 0.0f,   0.0f, 0.05f, 1.0f,   0.0433f, -0.025f, 1.0f,  // 4, 5, 6, 7
  };
  float uv[TEXCOORDS_COUNT * 2] = {
    0,1,  1.0f,1,  1.0f,0,  0,0,
    0,1,  1.0f,1,  1.0f,0,  0,0,
  };
  for (int i = 0; i < VERTICES_COUNT * 3 ; i++) r.mesh.vertices[i]  = v[i];

  // Normals for each face (calculate only one to apply to all 4)
  for (int j = 0; j < VERTICES_COUNT / 4; j++) {
    int normal_base = j*3 * 4; // Get index for the first of the 4 vertices
    Vector3 p  = { r.mesh.vertices[normal_base    + 0], r.mesh.vertices[normal_base    + 1], r.mesh.vertices[normal_base    + 2] };
    Vector3 p1 = { r.mesh.vertices[normal_base +1 + 0], r.mesh.vertices[normal_base +1 + 1], r.mesh.vertices[normal_base +1 + 2] };
    Vector3 p2 = { r.mesh.vertices[normal_base +2 + 0], r.mesh.vertices[normal_base +2 + 1], r.mesh.vertices[normal_base +2 + 2] };

    Vector3 edge1  = Vector3Subtract(p1, p);
    Vector3 edge2  = Vector3Subtract(p2, p);
    Vector3 normal = Vector3Normalize(Vector3CrossProduct(edge1, edge2));

    // each x-y-z per face
    for (int k = 0; k < 3; k++)
    {
      float applied_final = k % 3 == 0 ? normal.x : (k % 3 == 1 ? normal.y : normal.z );
      r.mesh.normals[normal_base    + k] = applied_final;
      r.mesh.normals[normal_base +1 + k] = applied_final;
      r.mesh.normals[normal_base +2 + k] = applied_final;
      r.mesh.normals[normal_base +3 + k] = applied_final;
    };
  };
  for (int i = 0; i < TEXCOORDS_COUNT * 2; i++) r.mesh.texcoords[i] = uv[i];

  // Separate starting index for the indices (dont take xyz into account)
  for (int face = 0; face < 2; face++)
  {
    int base  = face * 4;
    int idx_i = face * 6;
    r.mesh.indices[idx_i + 0] = base + 0;
    r.mesh.indices[idx_i + 1] = base + 1;
    r.mesh.indices[idx_i + 2] = base + 2;
    r.mesh.indices[idx_i + 3] = base + 0;
    r.mesh.indices[idx_i + 4] = base + 2;
    r.mesh.indices[idx_i + 5] = base + 3;
  }

  UploadMesh(&r.mesh, false);

  r.material = LoadMaterialDefault();
  r.material.maps[MATERIAL_MAP_DIFFUSE].texture = *texture;
  r.material.shader = *shader;
  renderable_set_transforms(&r, (Matrix[]){ MatrixIdentity() }, 1);

  return r;
}

MeshRenderable arc_generate_shadow_mesh(Shader *shader)
{
  // 3--2
  // |  |
  // |  |
  // 0--1

  // Per mesh
  const int VERTICES_COUNT  = 4;
  const int TRIANGLE_COUNT  = 2;
  const int TEXCOORDS_COUNT = 4;
  const int INDICES_COUNT   = TRIANGLE_COUNT * 3;

  // Initial vertices setup
  float initial_v[VERTICES_COUNT * 3] = {
    -0.0433f, 0.0f, 0.0f,   0.0433f, 0.0f, 0.0f,
     0.0433f, 0.0f, 1.0f,  -0.0433f, 0.0f, 1.0f,
  };
  float uv[TEXCOORDS_COUNT * 2] = {
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
  };
  unsigned short idx[INDICES_COUNT] = {0, 1, 2, 0, 2, 3};

  MeshRenderable r = { 0 };

  r.mesh.vertexCount   = VERTICES_COUNT;
  r.mesh.triangleCount = TRIANGLE_COUNT;

  r.mesh.vertices  =          (float *)malloc(VERTICES_COUNT  * 3 * sizeof(float));
  r.mesh.texcoords =          (float *)malloc(TEXCOORDS_COUNT * 2 * sizeof(float));
  r.mesh.indices   = (unsigned short *)malloc(INDICES_COUNT   * sizeof(unsigned short));

  for (int i = 0  ; i < VERTICES_COUNT * 3 ; i++) r.mesh.vertices[i]  = initial_v[i];
  for (int t = 0  ; t < TEXCOORDS_COUNT * 2; t++) r.mesh.texcoords[t] = uv[t];
  for (int ind = 0; ind < INDICES_COUNT  ; ind++) r.mesh.indices[ind] = idx[ind];

  UploadMesh(&r.mesh, false);

  r.material = LoadMaterialDefault();
  r.material.shader = *shader;

  renderable_set_transforms(&r, (Matrix[]){ MatrixIdentity() }, 1);

  return r;
}

MeshRenderable arc_generate_head_mesh(Texture2D *texture, Shader *shader)
{
  //   2 5
  //   /|\   .
  //  0 | 3
  //   \|/
  //   1 4

  // Per mesh
  const int VERTICES_COUNT  = 6;
  const int TRIANGLE_COUNT  = 2;
  const int TEXCOORDS_COUNT = 6;
  const int INDICES_COUNT   = TRIANGLE_COUNT * 3;

  MeshRenderable r = { 0 };

  r.mesh.vertexCount   = VERTICES_COUNT;
  r.mesh.triangleCount = TRIANGLE_COUNT;

  r.mesh.vertices  =          (float *)malloc(VERTICES_COUNT  * 3 * sizeof(float));
  r.mesh.normals   =          (float *)malloc(VERTICES_COUNT  * 3 * sizeof(float));
  r.mesh.texcoords =          (float *)malloc(TEXCOORDS_COUNT * 2 * sizeof(float));
  r.mesh.indices   = (unsigned short *)malloc(INDICES_COUNT * sizeof(unsigned short));

  double start_fp = 0;
  double end_fp = 0;
  float world_x_head, world_y_head;

  float head_v[VERTICES_COUNT * 3] = {
    -0.0433f, -0.025f, 0.0f,    0.0f, -0.03f, 0.07f,    0.0f, 0.05f, 0.0f,
     0.0433f, -0.025f, 0.0f,    0.0f, -0.03f, 0.07f,    0.0f, 0.05f, 0.0f,
  };
  for (int v = 0; v < VERTICES_COUNT * 3; v++) r.mesh.vertices[v] = head_v[v];
  for (int face = 0; face < 2; face++)
  {
    int vbase = face * 3 * 3; // 3 vertices per triangle, no sharing between the two flaps
    Vector3 p  = { r.mesh.vertices[vbase + 0], r.mesh.vertices[vbase + 1], r.mesh.vertices[vbase + 2] };
    Vector3 p1 = { r.mesh.vertices[vbase + 3], r.mesh.vertices[vbase + 4], r.mesh.vertices[vbase + 5] };
    Vector3 p2 = { r.mesh.vertices[vbase + 6], r.mesh.vertices[vbase + 7], r.mesh.vertices[vbase + 8] };

    Vector3 edge1  = Vector3Subtract(p1, p);
    Vector3 edge2  = Vector3Subtract(p2, p);
    Vector3 normal = Vector3Normalize(Vector3CrossProduct(edge1, edge2));

    for (int v = 0; v < 3; v++)
    {
      r.mesh.normals[vbase + v*3 + 0] = normal.x;
      r.mesh.normals[vbase + v*3 + 1] = normal.y;
      r.mesh.normals[vbase + v*3 + 2] = normal.z;
    }
  }

  float head_uv[TEXCOORDS_COUNT * 2] = {
    0.0f, 0.0f,   0.0f, 1.0f,   1.0f, 0.0f,
    1.0f, 0.0f,   1.0f, 1.0f,   0.0f, 0.0f,
  };
  for (int t = 0; t < TEXCOORDS_COUNT * 2; t++) r.mesh.texcoords[t] = head_uv[t];
  for (int i = 0; i < INDICES_COUNT; i++) r.mesh.indices[i] = i;

  UploadMesh(&r.mesh, false);

  r.material = LoadMaterialDefault();
  r.material.maps[MATERIAL_MAP_DIFFUSE].texture = *texture;
  r.material.shader = *shader;
  renderable_set_transforms(&r, (Matrix[]){ MatrixIdentity() }, 1);
  return r;
}

int arc_segment_render_compare_start_fp_asc(const void *a, const void *b)
{
  const ArcSegmentRender *segment_a = (const ArcSegmentRender *) a;
  const ArcSegmentRender *segment_b = (const ArcSegmentRender *) b;
  double sfp_a = segment_a->start_fp;
  double sfp_b = segment_b->start_fp;
  if (fabs(sfp_a - sfp_b) > 1e-6)
    return sfp_a < sfp_b ? -1 : 1;
  return 0;
}

void arc_segment_render_build_tree(ItvTree *tree, List *list, int low, int high)
{
  if (low > high) return;
  int mid = (low + high) / 2;
  ArcSegmentRender *segment_render = (ArcSegmentRender *)list_get(list, mid);
  Interval i = { .low = &segment_render->start_fp, .high = &segment_render->end_fp };
  itv_tree_insert(tree, segment_render, i);
  arc_segment_render_build_tree(tree, list, low, mid - 1);
  arc_segment_render_build_tree(tree, list, mid + 1, high);
}

static void segment_set_shader_values(ChartTimingGroup *tg, struct Arc *arc, ArcShader *arc_shader,
                                       float current_ms, float curr_bpm, Vector4 *tint_low, Vector4 *tint_high)
{
  bool arc_prop_validate = !tg->props.no_clip;
  if (!arc->is_void) arc_prop_validate = arc_prop_validate && tg->props.no_input;
  int should_clip_shader = (arc_prop_validate && arc->start_timing - current_ms <= 0) ? 1 : 0;
  int negative_bpm_shader = curr_bpm < 0.0f;
  SetShaderValue(arc_shader->shader, arc_shader->shouldClip_loc , &should_clip_shader , SHADER_UNIFORM_INT);
  SetShaderValue(arc_shader->shader, arc_shader->negativeBPM_loc, &negative_bpm_shader, SHADER_UNIFORM_INT);
  SetShaderValue(arc_shader->shader, arc_shader->tintLow_loc , tint_low , SHADER_UNIFORM_VEC4);
  SetShaderValue(arc_shader->shader, arc_shader->tintHigh_loc, tint_high, SHADER_UNIFORM_VEC4);
}

void draw_arc_shadow(ChartTimingGroup *tg, struct Arc *arc, ArcShader *arc_shader, MeshRenderable *mesh_r,
                     float current_ms, float curr_bpm, double z_pos, float add_fade, Matrix tr)
{
  float fade_ratio = arc->is_void ? 1.0f : ((z_pos - SKY_STOP_FADE) / (SHADOW_START_FADE - SKY_STOP_FADE));
  Vector4 shadow_tint = ColorNormalize(Fade(color_from_rgba(NOTE_SHADOW_LIGHT_CL), Clamp(fade_ratio, 0.0f, ARC_SHADOW_ALPHA * add_fade)));
  segment_set_shader_values(tg, arc, arc_shader, current_ms, curr_bpm, &shadow_tint, &shadow_tint);
  DrawMesh(mesh_r->mesh, mesh_r->material, tr);
}

void draw_arc_head(ChartTimingGroup *tg, struct Arc *arc, ArcShader *arc_shader, MeshRenderable *mesh_r,
                   float current_ms, float curr_bpm, float base_bpm, float scroll_speed, double curr_fp, float add_fade, bool colorblind)
{
  double z_pos = floor_position_to_z(arc->start_fp - curr_fp, base_bpm, scroll_speed);
  float  x_pos = arc_x_to_world(arc->x1);
  float  y_pos = arc_y_to_world(arc->y1);
  float  head_scale = arc->is_void ? TRACE_MESH_SCALE : ARC_MESH_SCALE;
  float  fade_ratio = arc->is_void ? 1.0f : (z_pos - SKY_STOP_FADE) / (SKY_START_FADE - SKY_STOP_FADE);

  Vector4 tint_low, tint_high;
  if (arc->is_void)
  {
    tint_low = tint_high = ColorNormalize(Fade(color_from_rgba(TRACE_CL), Clamp(fade_ratio, 0.0f, TRACE_ALPHA * add_fade)));
  } else
  {
    float final_alpha = ARC_ALPHA;
    struct Arc* arc_frfr = arc_get_firstmost_arc(arc);
    if (arc_frfr->start_timing - current_ms <= 0) final_alpha *= 0.72f;
    final_alpha *= add_fade;
    tint_low  = ColorNormalize(Fade(arc_get_color_low(arc->color, colorblind) , Clamp(fade_ratio, 0.0f, final_alpha)));
    tint_high = ColorNormalize(Fade(arc_get_color_high(arc->color, colorblind), Clamp(fade_ratio, 0.0f, final_alpha)));
  }
  segment_set_shader_values(tg, arc, arc_shader, current_ms, curr_bpm, &tint_low, &tint_high);
  DrawMesh(mesh_r->mesh, mesh_r->material, MatrixMultiply(MatrixScale(head_scale, head_scale, head_scale),
                                                          MatrixTranslate(x_pos, y_pos, z_pos)));
}

void draw_arc_segment(ChartTimingGroup *tg, struct Arc *arc, ArcShader *arc_shader, MeshRenderable *mesh_r,
                      float current_ms, float curr_bpm, double z_pos, float add_fade, bool colorblind, Matrix tr)
{
  float fade_ratio = arc->is_void ? 1.0f : (z_pos - SKY_STOP_FADE) / (SKY_START_FADE - SKY_STOP_FADE);

  Vector4 tint_low, tint_high;
  if (arc->is_void)
  {
    tint_low = tint_high = ColorNormalize(Fade(color_from_rgba(TRACE_CL), Clamp(fade_ratio, 0.0f, TRACE_ALPHA * add_fade)));
  } else
  {
    float final_alpha = ARC_ALPHA;
    struct Arc* arc_frfr = arc_get_firstmost_arc(arc);
    if (arc_frfr->start_timing - current_ms <= 0) final_alpha *= 0.72f;
    final_alpha *= add_fade;
    tint_low  = ColorNormalize(Fade(arc_get_color_low(arc->color, colorblind) , Clamp(fade_ratio, 0.0f, final_alpha)));
    tint_high = ColorNormalize(Fade(arc_get_color_high(arc->color, colorblind), Clamp(fade_ratio, 0.0f, final_alpha)));
  }
  segment_set_shader_values(tg, arc, arc_shader, current_ms, curr_bpm, &tint_low, &tint_high);
  DrawMesh(mesh_r->mesh, mesh_r->material, tr);
}

void draw_height_indicator(ArcSegmentRender *segment_render, struct Arc *arc, Mesh *mesh, Material mat, double z_pos, float add_fade, bool colorblind)
{
  if (!should_draw_height_indicator(segment_render, arc)) return;

  float fade_ratio = (z_pos - SKY_STOP_FADE) / (SKY_START_FADE - SKY_STOP_FADE);
  float arc_world_x = arc_x_to_world(arc->x1);
  float arc_world_y = arc_y_to_world(arc->y1);
  Matrix tr = MatrixMultiply(MatrixScale(1.0f, arc_world_y, 1.0f),
                             MatrixTranslate(arc_world_x, arc_world_y * 0.5f, z_pos));

  rlDisableDepthMask();
  mat.maps->color = Fade(arc_get_color_high(arc->color, colorblind), Clamp(fade_ratio, 0.0f, ARC_ALPHA * add_fade));
  DrawMesh(*mesh, mat, MatrixMultiply(MatrixRotateX(-90.0f * DEG2RAD), tr));
  rlEnableDepthMask();
}

void draw_arccap(ArcSegmentRender *segment_render, struct Arc *arc, Mesh *mesh, Material mat, float scale, float alpha, float current_ms)
{
  float arccap_scale = arc->is_void ? ARCCAP_TRACE_SCALE : ARCCAP_ARC_SCALE;
  arccap_scale *= scale;

  float arccap_start_x = arc_world_x_at(segment_render->start_timing, arc);
  float arccap_end_x   = arc_world_x_at(segment_render->end_timing  , arc);
  float arccap_start_y = arc_world_y_at(segment_render->start_timing, arc);
  float arccap_end_y   = arc_world_y_at(segment_render->end_timing  , arc);
  float lerp = 0.0;
  if (segment_render->start_timing != segment_render->end_timing)
    lerp = (current_ms - segment_render->start_timing) / (segment_render->end_timing - segment_render->start_timing);
  float lerp_x = Clamp(Lerp(arccap_start_x, arccap_end_x, lerp),
                       fminf(arccap_start_x, arccap_end_x),
                       fmaxf(arccap_start_x, arccap_end_x));
  float lerp_y = Clamp(Lerp(arccap_start_y, arccap_end_y, lerp),
                       fminf(arccap_start_y, arccap_end_y),
                       fmaxf(arccap_start_y, arccap_end_y));
  mat.maps->color = ColorAlpha(mat.maps->color, alpha);
  Matrix tr = MatrixMultiply(MatrixScale(arccap_scale, arccap_scale, 1.0f), MatrixTranslate(lerp_x, lerp_y, 0.0f));
  DrawMesh(*mesh, mat, tr);
}

bool should_draw_height_indicator(ArcSegmentRender *segment_render, struct Arc *arc)
{
  return !arc->is_void &&
         fabs(segment_render->start_fp - arc->start_fp) < 1e-6 &&
         (arc->is_head || fabsf(arc->y1 - arc->y2) > 1e-6);
}
