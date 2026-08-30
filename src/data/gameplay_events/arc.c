#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "constants.h"
#include "data/custom_types/custom_types.h"
#include "data/gameplay_events/arctap.h"
#include "raylib.h"
#include "raymath.h"
#include "render/mesh_renderable.h"
#include "render/playfield/playfield_services.h"
#include "rlgl.h"
#include "arc.h"
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
  const Arc *arc_a = (const Arc *) a;
  const Arc *arc_b = (const Arc *) b;
  return arc_a->start_timing - arc_b->start_timing;
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

// ARC SEGMENT
void arc_segment_generate_mesh(List *arc_segments_list, Arc *arc, Texture2D *texture, List *timing_events, RenderContext *render_ctx)
{
  int arc_duration      = arc->end_timing - arc->start_timing;
  float segment_length  = calculate_arc_segment_length(arc_duration, arc->arc_res);
  int segment_count     = (int)ceilf(arc_duration / segment_length);
  segment_count         = (int)fmax(segment_count, 1);

  // WARNING:
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
  for (int i = 0; i < VERTICES_COUNT * 3; i++) if (i % 3 != 2) { initial_v[i] *= arc_scale; }

  // UV setup
  float uv[TEXCOORDS_COUNT * 2] = {
    0,1,  0.5f,1,  0.5f,0,  0,0,
    0,1,  0.5f,1,  0.5f,0,  0,0,
  };

  // NOTE: Create mesh at z=0 (in timing) to prevent texture/rendering from going apeshit
  float curr_timing = 0.0f;
  Arc temp_arc = {
    .x1 = arc->x1, .y1 = arc->y1,
    .x2 = arc->x2, .y2 = arc->y2,
    .start_timing  = 0,
    .end_timing    = arc->end_timing - arc->start_timing,
    .type          = arc->type
  };
  for (int i = 0; i < segment_count; i++)
  {
    MeshRenderable r = { 0 };

    r.mesh.vertexCount   = VERTICES_COUNT;
    r.mesh.triangleCount = TRIANGLE_COUNT;

    r.mesh.vertices  =          (float *)malloc(VERTICES_COUNT  * 3 * sizeof(float));
    r.mesh.normals   =          (float *)malloc(VERTICES_COUNT  * 3 * sizeof(float));
    r.mesh.texcoords =          (float *)malloc(TEXCOORDS_COUNT * 2 * sizeof(float));
    r.mesh.indices   = (unsigned short *)malloc(INDICES_COUNT * sizeof(unsigned short));

    // Starting index for each segment (index * vertices * xyz coords)
    float increment = fminf(arc->end_timing - arc->start_timing - curr_timing, segment_length);
    float start_fp = 0;
    float end_fp = 0;

    // Vertices for each face
    for (int j = 0; j < VERTICES_COUNT; j++)
    {
      // condition match => front of arc
      bool is_front_of_arc = j == 0 || j == 1 || j == 4 || j == 5;
      float target_timing = is_front_of_arc ? curr_timing : curr_timing + increment;
      float world_x = arc_world_x_at(target_timing, &temp_arc, is_front_of_arc ? arc->x1 : arc->x2);
      float world_y = arc_world_y_at(target_timing, &temp_arc, is_front_of_arc ? arc->y1 : arc->y2);
      if (is_front_of_arc) start_fp = get_floor_position(timing_events, target_timing + arc->start_timing);
      if (!is_front_of_arc)  end_fp = get_floor_position(timing_events, target_timing + arc->start_timing);

      // Get the correct z scaling base on the arc data, then offset world_z back to origin by the arc's start_timing
      float segment_start_timing = curr_timing + arc->start_timing;
      float z_target  = floor_position_to_z(get_floor_position(timing_events, target_timing + arc->start_timing),
                                  render_ctx->chart_settings.base_bpm, render_ctx->chart_settings.scroll_speed);
      float z_segment = floor_position_to_z(get_floor_position(timing_events, segment_start_timing),
                                  render_ctx->chart_settings.base_bpm, render_ctx->chart_settings.scroll_speed);
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
    r.material.shader = render_ctx->arc_clip_shader.shader;
    set_mesh_transforms(&r, (Matrix[]){ MatrixIdentity() }, 1);

    ArcSegment *sgm = (ArcSegment *)list_get(arc_segments_list, arc_segments_list->size - segment_count + i);
    sgm->mesh_r = r;
    sgm->start_fp = start_fp;
    sgm->end_fp = end_fp;

    curr_timing += increment;
  }
}

void arc_segment_with_head_generate_mesh(List *arc_segments_list, Arc *arc, Texture2D *texture, List *timing_events, RenderContext *render_ctx)
{
  int arc_duration      = arc->end_timing - arc->start_timing;
  float segment_length  = calculate_arc_segment_length(arc_duration, arc->arc_res);
  int segment_count     = (int)ceilf(arc_duration / segment_length);
  segment_count         = (int)fmax(segment_count, 1);

  // WARNING:
  //   2     6
  //  /|     |\
  // 3 |     | 7
  // | 1     5 |
  // |/ 10 13 \|
  // 0   /|\   4
  //    8 | 11
  //     \|/
  //     9 12

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
  for (int i = 0; i < VERTICES_COUNT * 3; i++) if (i % 3 != 2) { initial_v[i] *= arc_scale; }

  // UV setup
  float uv[TEXCOORDS_COUNT * 2] = {
    0,1,  0.5f,1,  0.5f,0,  0,0,
    0,1,  0.5f,1,  0.5f,0,  0,0,
  };

  // NOTE: Create mesh at z=0 (in timing) to prevent texture/rendering from going apeshit
  float curr_timing = 0.0f;
  Arc temp_arc = {
    .x1 = arc->x1, .y1 = arc->y1,
    .x2 = arc->x2, .y2 = arc->y2,
    .start_timing  = 0,
    .end_timing    = arc->end_timing - arc->start_timing,
    .type          = arc->type
  };
  for (int i = 0; i < segment_count; i++)
  {
    MeshRenderable r = { 0 };

    r.mesh.vertexCount   = VERTICES_COUNT;
    r.mesh.triangleCount = TRIANGLE_COUNT;

    r.mesh.vertices  =          (float *)malloc(VERTICES_COUNT  * 3 * sizeof(float));
    r.mesh.normals   =          (float *)malloc(VERTICES_COUNT  * 3 * sizeof(float));
    r.mesh.texcoords =          (float *)malloc(TEXCOORDS_COUNT * 2 * sizeof(float));
    r.mesh.indices   = (unsigned short *)malloc(INDICES_COUNT * sizeof(unsigned short));

    // Starting index for each segment (index * vertices * xyz coords)
    float increment = fminf(arc->end_timing - arc->start_timing - curr_timing, segment_length);
    float start_fp = 0;
    float end_fp = 0;

    // Vertices for each face
    for (int j = 0; j < VERTICES_COUNT; j++)
    {
      // condition match => front of arc
      bool is_front_of_arc = j == 0 || j == 1 || j == 4 || j == 5;
      float target_timing = is_front_of_arc ? curr_timing : curr_timing + increment;
      float world_x = arc_world_x_at(target_timing, &temp_arc, is_front_of_arc ? arc->x1 : arc->x2);
      float world_y = arc_world_y_at(target_timing, &temp_arc, is_front_of_arc ? arc->y1 : arc->y2);
      if (is_front_of_arc) start_fp = get_floor_position(timing_events, target_timing + arc->start_timing);
      if (!is_front_of_arc)  end_fp = get_floor_position(timing_events, target_timing + arc->start_timing);

      // Get the correct z scaling base on the arc data, then offset world_z back to origin by the arc's start_timing
      float segment_start_timing = curr_timing + arc->start_timing;
      float z_target  = floor_position_to_z(get_floor_position(timing_events, target_timing + arc->start_timing),
                                  render_ctx->chart_settings.base_bpm, render_ctx->chart_settings.scroll_speed);
      float z_segment = floor_position_to_z(get_floor_position(timing_events, segment_start_timing),
                                  render_ctx->chart_settings.base_bpm, render_ctx->chart_settings.scroll_speed);
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
    r.material.shader = render_ctx->arc_clip_shader.shader;
    set_mesh_transforms(&r, (Matrix[]){ MatrixIdentity() }, 1);

    ArcSegment *sgm = (ArcSegment *)list_get(arc_segments_list, arc_segments_list->size - segment_count + i);
    sgm->mesh_r = r;
    sgm->start_fp = start_fp;
    sgm->end_fp = end_fp;

    curr_timing += increment;
  }
}

void shadow_segment_generate_mesh(List *arc_segments_list, Arc *arc, List *timing_events, RenderContext *render_ctx)
{
  int arc_duration      = arc->end_timing - arc->start_timing;
  float segment_length  = calculate_arc_segment_length(arc_duration, arc->arc_res);
  int segment_count     = (int)ceilf(arc_duration / segment_length);
  segment_count         = (int)fmax(segment_count, 1);

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
  float arc_scale = arc->is_void ? TRACE_MESH_SCALE : ARC_MESH_SCALE;
  for (int i = 0; i < VERTICES_COUNT * 3; i++) if (i % 3 != 2) { initial_v[i] *= arc_scale; }

  // NOTE: Create mesh at z=0 (in timing) to prevent texture/rendering from going apeshit
  float curr_timing = 0.0f;
    Arc temp_arc = {
      .x1 = arc->x1, .y1 = arc->y1,
      .x2 = arc->x2, .y2 = arc->y2,
      .start_timing  = 0,
      .end_timing    = arc->end_timing - arc->start_timing,
      .type          = arc->type
    };
  for (int i = 0; i < segment_count; i++)
  {
    MeshRenderable r = { 0 };

    r.mesh.vertexCount   = VERTICES_COUNT;
    r.mesh.triangleCount = TRIANGLE_COUNT;

    r.mesh.vertices  =          (float *)malloc(VERTICES_COUNT* 3 * sizeof(float));
    r.mesh.indices   = (unsigned short *)malloc(INDICES_COUNT * sizeof(unsigned short));

    // Starting index for each segment (index * vertices * xyz coords)
    float increment = fminf(arc->end_timing - arc->start_timing - curr_timing, segment_length);

    // Vertices for each face
    for (int j = 0; j < VERTICES_COUNT; j++)
    {
      // condition match => front of arc
      bool is_front_of_arc = j == 0 || j == 1;
      float target_timing = is_front_of_arc ? curr_timing : curr_timing + increment;
      float world_x = arc_world_x_at(target_timing, &temp_arc, is_front_of_arc ? arc->x1 : arc->x2);

      // Get the correct z scaling base on the arc data, then offset world_z back to origin by the arc's start_timing
      float segment_start_timing = curr_timing + arc->start_timing;
      float z_target  = floor_position_to_z(get_floor_position(timing_events, target_timing + arc->start_timing),
                                  render_ctx->chart_settings.base_bpm, render_ctx->chart_settings.scroll_speed);
      float z_segment = floor_position_to_z(get_floor_position(timing_events, segment_start_timing),
                                  render_ctx->chart_settings.base_bpm, render_ctx->chart_settings.scroll_speed);
      float world_z  = z_target - z_segment;

      // each x-y-z per vertex
      for (int k = 0; k < 3; k++)
      {
        float applied_final = k % 3 == 0 ? world_x : ( k % 3 == 1 ? 0.0f : world_z );
        if (k % 3 == 2) r.mesh.vertices[j*3 + k] = initial_v[j*3 + k] * -applied_final; // Scale Z by scalar
        else            r.mesh.vertices[j*3 + k] = initial_v[j*3 + k] +  applied_final; // Offset XY by addition
      };
    };

    r.mesh.indices[0] = 0;
    r.mesh.indices[1] = 1;
    r.mesh.indices[2] = 2;
    r.mesh.indices[3] = 0;
    r.mesh.indices[4] = 2;
    r.mesh.indices[5] = 3;

    UploadMesh(&r.mesh, false);

    r.material = LoadMaterialDefault();
    r.material.maps[MATERIAL_MAP_DIFFUSE].color = (Color){ 90, 90, 90, 55 };
    r.material.shader = render_ctx->arc_clip_shader.shader;

    set_mesh_transforms(&r, (Matrix[]){ MatrixIdentity() }, 1);

    ArcSegment *sgm = (ArcSegment *)list_get(arc_segments_list, arc_segments_list->size - (segment_count - i));
    sgm->shadow_r = r;

    curr_timing += increment;
  }
}

int arc_segment_compare_start_fp_asc(const void *a, const void *b)
{
  const ArcSegment *segment_a = (const ArcSegment *) a;
  const ArcSegment *segment_b = (const ArcSegment *) b;
  if (segment_a->start_fp < segment_b->start_fp) return -1;
  else return 1;
}

