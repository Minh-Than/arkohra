#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include "arc.h"
#include "constants.h"
#include "color_services.h"
#include "data/custom_types/custom_types.h"
#include "data/gameplay_events/arctap.h"
#include "gameplay/arc_formula.h"
#include "render/mesh_renderable.h"

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
  const Arc *arc_a = (const Arc *) a;
  const Arc *arc_b = (const Arc *) b;
  return arc_a->start_timing - arc_b->start_timing;
}

MeshRenderable generate_arccap_mesh(Texture2D *texture)
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

MeshRenderable generate_arc_height_mesh(Texture2D *texture)
{
  Mesh height_indicator_mesh = GenMeshPlane(0.5f, 1, 1, 1);
  UploadMesh(&height_indicator_mesh, false);
  Material height_indicator_material = LoadMaterialDefault();
  height_indicator_material.maps[MATERIAL_MAP_DIFFUSE].texture = *texture;

  MeshRenderable r = { .mesh = height_indicator_mesh, .material = height_indicator_material };
  renderable_set_transforms(&r, (Matrix[]){MatrixIdentity()}, 1);

  return r;
}

void arc_print(const void *elem)
{
  const Arc *arc = (const Arc *)elem;
  printf("arc(%d,%d,%.2f,%.2f,%d,%.2f,%.2f,%d,%s,%d)",
         arc->start_timing, arc->end_timing,
         arc->x1, arc->x2, arc->type, arc->y1, arc->y2,
         arc->color, arc->sfx, arc->is_void);

  bool has_arctaps = true;
  for (int i = 0; i < arc->arctaps.size; i++)
  {
    if(i == 0) printf("[");

    ArcTap *arctap = (ArcTap *)list_get((List *)&arc->arctaps, i);
    printf("arctap(%d)", arctap->timing);

    if (i < arc->arctaps.size - 1) printf(",");
  }
  if (arc->arctaps.size != 0) printf("]");
  printf(";");
}

MeshRenderable generate_arc_body_mesh(ChartSettings *chart_settings, List *timing_events, Arc *arc,
                                      Texture2D *texture, Shader *shader, float curr_timing, float increment)
{
  //   2 6
  //  /| |\
  // 3 | | 7
  // | 1 5 |
  // |/   \|
  // 0     4

  // Per mesh
  const int VERTICES_COUNT  = 8;
  const int TRIANGLE_COUNT  = 4;
  const int TEXCOORDS_COUNT = 8;
  const int INDICES_COUNT   = TRIANGLE_COUNT * 3;

  // Initial vertices setup
  float initial_v[VERTICES_COUNT * 3] = {
    -0.0433f, -0.025f, -1.0f,   0.0f, 0.05f, -1.0f,   0.0f, 0.05f, -1.0f,  -0.0433f, -0.025f, -1.0f,  // 0, 1, 2, 3
     0.0433f, -0.025f, -1.0f,   0.0f, 0.05f, -1.0f,   0.0f, 0.05f, -1.0f,   0.0433f, -0.025f, -1.0f,  // 4, 5, 6, 7
  };
  float arc_scale = arc->is_void ? TRACE_MESH_SCALE : ARC_MESH_SCALE;
  for (int j = 0; j < VERTICES_COUNT * 3; j++) if (j % 3 != 2) { initial_v[j] *= arc_scale; }

  // UV setup
  float uv[TEXCOORDS_COUNT * 2] = {
    0,1,  1.0f,1,  1.0f,0,  0,0,
    0,1,  1.0f,1,  1.0f,0,  0,0,
  };

  Arc temp_arc = {
    .x1 = arc->x1, .y1 = arc->y1,
    .x2 = arc->x2, .y2 = arc->y2,
    .start_timing  = 0,
    .end_timing    = arc->end_timing - arc->start_timing,
    .type          = arc->type
  };

  MeshRenderable r = { 0 };

  r.mesh.vertexCount   = VERTICES_COUNT;
  r.mesh.triangleCount = TRIANGLE_COUNT;

  r.mesh.vertices  =          (float *)malloc(VERTICES_COUNT  * 3 * sizeof(float));
  r.mesh.normals   =          (float *)malloc(VERTICES_COUNT  * 3 * sizeof(float));
  r.mesh.texcoords =          (float *)malloc(TEXCOORDS_COUNT * 2 * sizeof(float));
  r.mesh.indices   = (unsigned short *)malloc(INDICES_COUNT * sizeof(unsigned short));

  // Vertices for each face
  for (int j = 0; j < VERTICES_COUNT; j++)
  {
    // condition match => front of arc
    bool is_front_of_arc = j == 0 || j == 1 || j == 4 || j == 5;
    float target_timing = is_front_of_arc ? curr_timing : curr_timing + increment;
    bool zero_duration = (arc->end_timing == arc->start_timing);
    float world_x = zero_duration
      ? arc_x_to_world(is_front_of_arc ? arc->x1 : arc->x2)
      : arc_world_x_at(target_timing, &temp_arc);
    float world_y = zero_duration
      ? arc_y_to_world(is_front_of_arc ? arc->y1 : arc->y2)
      : arc_world_y_at(target_timing, &temp_arc);

    // Get the correct z scaling base on the arc data, then offset world_z back to origin by the arc's start_timing
    float segment_start_timing = curr_timing + arc->start_timing;
    float z_target  = floor_position_to_z(get_floor_position(timing_events, target_timing + arc->start_timing),
                                          chart_settings->base_bpm,
                                          chart_settings->scroll_speed);
    float z_segment = floor_position_to_z(get_floor_position(timing_events, segment_start_timing),
                                          chart_settings->base_bpm,
                                          chart_settings->scroll_speed);
    float world_z  = z_target - z_segment;

    // each x-y-z per vertex
    for (int k = 0; k < 3; k++)
    {
      float applied_final = k % 3 == 0 ? world_x : ( k % 3 == 1 ? world_y : world_z );
      if (k % 3 == 2) r.mesh.vertices[j*3 + k] = initial_v[j*3 + k] * -applied_final; // Scale Z by scalar
      else            r.mesh.vertices[j*3 + k] = initial_v[j*3 + k] +  applied_final; // Offset XY by addition
    };
  };

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

  for (int t = 0; t < TEXCOORDS_COUNT * 2; t++) r.mesh.texcoords[t] = uv[t];

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

MeshRenderable generate_arc_shadow_mesh(ChartSettings *chart_settings, List *timing_events, Arc *arc,
                                        Shader *shader, float curr_timing, float increment)
{
  // WARNING:
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
    -0.0433f, 0.0f, -1.0f,   0.0433f, 0.0f, -1.0f,
     0.0433f, 0.0f, -1.0f,  -0.0433f, 0.0f, -1.0f,
  };
  float uv[TEXCOORDS_COUNT * 2] = {
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
  };
  unsigned short idx[INDICES_COUNT] = {0, 1, 2, 0, 2, 3};

  float arc_scale = arc->is_void ? TRACE_MESH_SCALE : ARC_MESH_SCALE;
  for (int v = 0; v < VERTICES_COUNT * 3; v++) if (v % 3 != 2) { initial_v[v] *= arc_scale; }

  // NOTE: Create mesh at z=0 (in timing) to prevent texture/rendering from going apeshit
  Arc temp_arc = {
    .x1 = arc->x1, .y1 = arc->y1,
    .x2 = arc->x2, .y2 = arc->y2,
    .start_timing  = 0,
    .end_timing    = arc->end_timing - arc->start_timing,
    .type          = arc->type
  };

  MeshRenderable r = { 0 };

  r.mesh.vertexCount   = VERTICES_COUNT;
  r.mesh.triangleCount = TRIANGLE_COUNT;

  r.mesh.vertices  =          (float *)malloc(VERTICES_COUNT  * 3 * sizeof(float));
  r.mesh.texcoords =          (float *)malloc(TEXCOORDS_COUNT * 2 * sizeof(float));
  r.mesh.indices   = (unsigned short *)malloc(INDICES_COUNT   * sizeof(unsigned short));

  // Vertices for each face
  for (int j = 0; j < VERTICES_COUNT; j++)
  {
    // condition match => front of arc
    bool is_front_of_arc = j == 0 || j == 1;
    float target_timing = is_front_of_arc ? curr_timing : curr_timing + increment;
    float world_x = arc_world_x_at(target_timing, &temp_arc);

    // Get the correct z scaling base on the arc data, then offset world_z back to origin by the arc's start_timing
    float segment_start_timing = curr_timing + arc->start_timing;
    float z_target  = floor_position_to_z(get_floor_position(timing_events, target_timing + arc->start_timing),
                                          chart_settings->base_bpm,
                                          chart_settings->scroll_speed);
    float z_segment = floor_position_to_z(get_floor_position(timing_events, segment_start_timing),
                                          chart_settings->base_bpm,
                                          chart_settings->scroll_speed);
    float world_z  = z_target - z_segment;

    // each x-y-z per vertex
    for (int k = 0; k < 3; k++)
    {
      float applied_final = k % 3 == 0 ? world_x : ( k % 3 == 1 ? 0.0f : world_z );
      if (k % 3 == 2) r.mesh.vertices[j*3 + k] = initial_v[j*3 + k] * -applied_final; // Scale Z by scalar
      else            r.mesh.vertices[j*3 + k] = initial_v[j*3 + k] +  applied_final; // Offset XY by addition
    };
  };

  for (int t = 0  ; t < TEXCOORDS_COUNT * 2; t++) r.mesh.texcoords[t] = uv[t];
  for (int ind = 0; ind < INDICES_COUNT; ind++) r.mesh.indices[ind] = idx[ind];

  UploadMesh(&r.mesh, false);

  r.material = LoadMaterialDefault();
  r.material.maps[MATERIAL_MAP_DIFFUSE].color = color_from_rgba(NOTE_SHADOW_CL);
  r.material.shader = *shader;

  renderable_set_transforms(&r, (Matrix[]){ MatrixIdentity() }, 1);

  return r;
}

void generate_segment_meshes(ChartSettings *chart_settings, ChartTimingGroup *tg, Arc *arc, Texture2D *texture, Shader *shader)
{
  int arc_duration      = arc->end_timing - arc->start_timing;
  float segment_length  = calculate_arc_segment_length(arc_duration, arc->arc_res * tg->props.arc_res);
  int segment_count     = (int)ceilf(arc_duration / segment_length);
  segment_count         = (int)fmax(segment_count, 1);

  List *timing_events = &tg->timing_events;
  float curr_timing = 0.0f;
  for (int i = 0; i < segment_count; i++)
  {
    float increment = fminf(arc->end_timing - arc->start_timing - curr_timing, segment_length);
    double start_fp = get_floor_position(timing_events, curr_timing + arc->start_timing);
    double end_fp = get_floor_position(timing_events, curr_timing + increment + arc->start_timing);

    ArcSegment arc_segment = { .arc = arc, .start_fp = start_fp, .end_fp = end_fp };
    arc_segment.mesh_r   = generate_arc_body_mesh  (chart_settings, timing_events, arc, texture, shader, curr_timing, increment);
    arc_segment.shadow_r = generate_arc_shadow_mesh(chart_settings, timing_events, arc, shader, curr_timing, increment);
    list_push(&tg->arc_segments, &arc_segment);

    curr_timing += increment;
  }
}

MeshRenderable generate_arc_head_mesh(Texture2D *texture, Shader *shader)
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

int arc_segment_compare_start_fp_asc(const void *a, const void *b)
{
  const ArcSegment *segment_a = (const ArcSegment *) a;
  const ArcSegment *segment_b = (const ArcSegment *) b;
  double sfp_a = segment_a->start_fp;
  double sfp_b = segment_b->start_fp;
  if (fabs(sfp_a - sfp_b) > 1e-6)
    return sfp_a < sfp_b ? -1 : 1;
  return 0;
}

int arc_segment_const_void_compare_start_fp_asc(const void *a, const void *b)
{
  const ArcSegment *segment_a = *(const ArcSegment *const *) a;
  const ArcSegment *segment_b = *(const ArcSegment *const *) b;
  double sfp_a = segment_a->start_fp;
  double sfp_b = segment_b->start_fp;
  if (fabs(sfp_a - sfp_b) > 1e-6)
    return sfp_a < sfp_b ? -1 : 1;
  return 0;
}

void arc_segment_build_tree(ItvTree *tree, List *list, int low, int high)
{
  if (low > high) return;
  int mid = (low + high) / 2;
  ArcSegment *arc_segment = (ArcSegment *)list_get(list, mid);
  Interval i = { .low = &arc_segment->start_fp, .high = &arc_segment->end_fp };
  itv_tree_insert(tree, arc_segment, i);
  arc_segment_build_tree(tree, list, low, mid - 1);
  arc_segment_build_tree(tree, list, mid + 1, high);
}

void draw_arc_shadow(ChartTimingGroup *tg, ArcSegment *arc_segment, ArcShader *arc_shader, float current_ms, float curr_bpm, float z_pos)
{
  Arc *arc = arc_segment->arc;
  Vector4 shadow_tint = ColorNormalize(color_from_rgba(NOTE_SHADOW_CL));
  bool arc_prop_validate = !tg->props.no_clip;
  if (!arc->is_void) arc_prop_validate = arc_prop_validate && tg->props.no_input;
  int should_clip_shader = (arc_prop_validate && arc->start_timing - current_ms <= 0) ? 1 : 0;
  int negative_bpm_shader = curr_bpm < 0.0f;
  SetShaderValue(arc_shader->shader, arc_shader->shouldClip_loc , &should_clip_shader , SHADER_UNIFORM_INT);
  SetShaderValue(arc_shader->shader, arc_shader->negativeBPM_loc, &negative_bpm_shader, SHADER_UNIFORM_INT);
  SetShaderValue(arc_shader->shader, arc_shader->tintLow_loc , &shadow_tint, SHADER_UNIFORM_VEC4);
  SetShaderValue(arc_shader->shader, arc_shader->tintHigh_loc, &shadow_tint, SHADER_UNIFORM_VEC4);
  DrawMesh(arc_segment->shadow_r.mesh, arc_segment->shadow_r.material, MatrixTranslate(0.0f, 0.0f, z_pos));
}

void draw_arc_head(ChartTimingGroup *tg, ArcSegment *arc_segment, ArcShader *arc_shader, MeshRenderable *mesh_r,
                   float current_ms, float curr_bpm, float base_bpm, float scroll_speed, double curr_fp)
{
  Arc *arc = arc_segment->arc;
  if (!arc->is_head) return;
  if (fabs(arc_segment->start_fp - arc->start_fp) > 1e-6) return;

  bool arc_prop_validate = !tg->props.no_clip;
  if (!arc->is_void) arc_prop_validate = arc_prop_validate && tg->props.no_input;
  int should_clip_shader = (arc_prop_validate && arc->start_timing - current_ms <= 0) ? 1 : 0;
  int negative_bpm_shader = curr_bpm < 0.0f;
  SetShaderValue(arc_shader->shader, arc_shader->shouldClip_loc , &should_clip_shader , SHADER_UNIFORM_INT);
  SetShaderValue(arc_shader->shader, arc_shader->negativeBPM_loc, &negative_bpm_shader, SHADER_UNIFORM_INT);

  Vector4 tint_low, tint_high;
  tint_low = tint_high = ColorNormalize(color_from_rgba(TRACE_CL)); // Default trace tint
  if (!arc->is_void)
  {
    tint_low  = ColorNormalize(color_from_rgba( arc->color == 0 ? ARC_BLUE_LOW_CL : ARC_PINK_LOW_CL));
    tint_high = ColorNormalize(color_from_rgba( arc->color == 0 ? ARC_BLUE_HIGH_CL : ARC_PINK_HIGH_CL));
  }
  SetShaderValue(arc_shader->shader, arc_shader->tintLow_loc , &tint_low , SHADER_UNIFORM_VEC4);
  SetShaderValue(arc_shader->shader, arc_shader->tintHigh_loc, &tint_high, SHADER_UNIFORM_VEC4);

  float x_pos = arc_x_to_world(arc->x1);
  float y_pos = arc_y_to_world(arc->y1);
  float z_pos = floor_position_to_z(arc->start_fp - curr_fp, base_bpm, scroll_speed);
  float head_scale = arc->is_void ? TRACE_MESH_SCALE : ARC_MESH_SCALE;
  Matrix tr = MatrixMultiply(MatrixScale(head_scale, head_scale, head_scale), MatrixTranslate(x_pos, y_pos, z_pos));
  DrawMesh(mesh_r->mesh, mesh_r->material, tr);
}

void draw_arc_segment(ChartTimingGroup *tg, ArcSegment *arc_segment, ArcShader *arc_shader, float current_ms, float curr_bpm, float z_pos)
{
  Arc *arc = arc_segment->arc;

  // Settings up shader
  Vector4 tint_low, tint_high;
  tint_low = tint_high = ColorNormalize(color_from_rgba(TRACE_CL)); // Default trace tint
  if (!arc->is_void)
  {
    tint_low  = ColorNormalize(color_from_rgba( arc->color == 0 ? ARC_BLUE_LOW_CL : ARC_PINK_LOW_CL));
    tint_high = ColorNormalize(color_from_rgba( arc->color == 0 ? ARC_BLUE_HIGH_CL : ARC_PINK_HIGH_CL));
  }
  bool arc_prop_validate = !tg->props.no_clip;
  if (!arc->is_void) arc_prop_validate = arc_prop_validate && tg->props.no_input;
  int should_clip_shader = (arc_prop_validate && arc->start_timing - current_ms <= 0) ? 1 : 0;
  int negative_bpm_shader = curr_bpm < 0.0f;
  SetShaderValue(arc_shader->shader, arc_shader->shouldClip_loc , &should_clip_shader , SHADER_UNIFORM_INT);
  SetShaderValue(arc_shader->shader, arc_shader->negativeBPM_loc, &negative_bpm_shader, SHADER_UNIFORM_INT);
  SetShaderValue(arc_shader->shader, arc_shader->tintLow_loc , &tint_low , SHADER_UNIFORM_VEC4);
  SetShaderValue(arc_shader->shader, arc_shader->tintHigh_loc, &tint_high, SHADER_UNIFORM_VEC4);

  DrawMesh(arc_segment->mesh_r.mesh, arc_segment->mesh_r.material, MatrixTranslate(0.0f, 0.0f, z_pos));
}

void draw_height_indicator(ArcSegment *arc_segment, Mesh *mesh, Material mat, float z_pos)
{
  if (!should_draw_height_indicator(arc_segment)) return;

  Arc *arc = arc_segment->arc;
  float arc_world_x = arc_x_to_world(arc->x1);
  float arc_world_y = arc_y_to_world(arc->y1);
  Matrix tr = MatrixMultiply(MatrixScale(1.0f, arc_world_y, 1.0f),
                             MatrixTranslate(arc_world_x, arc_world_y * 0.5f, z_pos));

  rlDisableDepthMask();
  mat.maps->color = arc->color == 0 ? color_from_rgba(ARC_BLUE_HIGH_CL) : color_from_rgba(ARC_PINK_HIGH_CL);
  DrawMesh(*mesh, mat, MatrixMultiply(MatrixRotateX(-90.0f * DEG2RAD), tr));
  rlEnableDepthMask();
}

void draw_arccap(ArcSegment *arc_segment, Mesh *mesh, Material mat, float scale, float alpha, float current_ms)
{
  Arc *arc = arc_segment->arc;

  float arccap_x = arc_world_x_at(current_ms, arc);
  float arccap_y = arc_world_y_at(current_ms, arc);
  float arccap_scale = arc->is_void ? ARCCAP_TRACE_SCALE : ARCCAP_ARC_SCALE;
  arccap_scale *= scale;
  mat.maps->color = ColorAlpha(mat.maps->color, alpha);
  Matrix tr = MatrixMultiply(MatrixScale(arccap_scale, arccap_scale, 1.0f), MatrixTranslate(arccap_x, arccap_y, 0.0f));
  DrawMesh(*mesh, mat, tr);
}

bool should_draw_height_indicator(ArcSegment *arc_segment)
{
  Arc *arc = arc_segment->arc;
  return !arc->is_void &&
         fabs(arc_segment->start_fp - arc->start_fp) < 1e-6 &&
         (arc->is_head || fabsf(arc->y1 - arc->y2) > 1e-6);
}
