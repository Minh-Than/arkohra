#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "tap.h"
#include "color_services.h"
#include "constants.h"
#include "raylib.h"
#include "raymath.h"
#include "render/mesh_renderable.h"
#include "render/utils/drawing.h"
#include "gameplay/arc_formula.h"

void tap_print(const void *elem)
{
  const Tap *tap = (const Tap *)elem;
  printf("(%d,%.2f)\n", tap->timing, tap->lane);
}

void draw_tap_connection(Tap *tap, ChartSettings *chart_settings, float base_bpm, float scroll_speed, double curr_fp, float add_fade)
{
  double z_pos   = floor_position_to_z(tap->fp - curr_fp, base_bpm, scroll_speed);
  float fade_ratio = (z_pos - TAP_STOP_FADE) / (TAP_START_FADE - TAP_STOP_FADE);
  for (int k = 0; k < tap->connector_x.size; k++)
  {
    float x = *(float *)list_get(&tap->connector_x, k);
    float y = *(float *)list_get(&tap->connector_y, k);
    DrawConnector((Vector3){ lane_to_world_x(tap->lane), 0.0f, z_pos },
                  (Vector3){ x, y - 0.21f, z_pos },
                  Lerp(0.05f, 0.08f, z_pos / -100.0f),
                  Fade(color_from_rgba(chart_settings->skin_side == SK_CONFLICT ? CONFLICT_CONNECTOR_CL
                                                                                : LIGHT_CONNECTOR_CL),
                       Clamp(fade_ratio, 0.0f, 1.0f * add_fade))
                  );
  }
}

MeshRenderable tap_load_mesh(Texture2D *texture)
{
  const int VERTICES_COUNT  = 4;
  const int TRIANGLE_COUNT  = 2;
  const int TEXCOORDS_COUNT = 4;
  const int INDICES_COUNT   = 3;

  MeshRenderable r = { 0 };

  r.mesh.vertexCount    = VERTICES_COUNT;
  r.mesh.triangleCount  = TRIANGLE_COUNT;

  r.mesh.vertices  =          (float *)malloc(VERTICES_COUNT * 3 * sizeof(float));
  r.mesh.texcoords =          (float *)malloc(TEXCOORDS_COUNT * 2 * sizeof(float));
  r.mesh.indices   = (unsigned short *)malloc(INDICES_COUNT * 2 * sizeof(unsigned short));

  float v[VERTICES_COUNT * 3] = {
    -1.08f, 0.0f, 0.0f,
     1.08f, 0.0f, 0.0f,
     1.08f, 0.0f, 1.0f,
    -1.08f, 0.0f, 1.0f,
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

void tap_parse_aff(List *taps, const char* line, int *current_tg)
{
  int timing;
  float lane;
  int matched = sscanf(line, "(%d,%f);", &timing, &lane);
  if (matched == 2)
  {
    List connector_x; list_init(&connector_x, sizeof(float));
    List connector_y; list_init(&connector_y, sizeof(float));
    Tap tap = (Tap) {
      .connector_x = connector_x,
      .connector_y = connector_y,
      .lane = lane,
      .timing = timing,
      .timing_group = *current_tg,
    };
    list_push(taps, &tap);
  }
}

int tap_compare_fp_asc(const void *a, const void *b)
{
  const Tap *tap_a = (const Tap *) a;
  const Tap *tap_b = (const Tap *) b;
  double fp_a = tap_a->fp;
  double fp_b = tap_b->fp;
  if (fabs(fp_a - fp_b) > 1e-6 && fp_a < fp_b) return -1;
  if (fabs(fp_a - fp_b) > 1e-6 && fp_a > fp_b) return 1;
  return 0;
}

int tap_compare_id_eq(const void *a, const void *b)
{
  const Tap *tap_a = (const Tap *) a;
  const Tap *tap_b = (const Tap *) b;
  return tap_a->id == tap_b->id ? 0 : -1;
}

int tap_render_compare_fp_asc(const void *a, const void *b)
{
  const TapRender *tap_a = (const TapRender *) a;
  const TapRender *tap_b = (const TapRender *) b;
  double fp_a = tap_a->fp;
  double fp_b = tap_b->fp;
  if (fabs(fp_a - fp_b) > 1e-6 && fp_a < fp_b) return -1;
  if (fabs(fp_a - fp_b) > 1e-6 && fp_a > fp_b) return 1;
  return 0;
}
