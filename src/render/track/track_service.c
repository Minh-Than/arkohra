#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "constants.h"
#include "track_service.h"
#include "gameplay/camera/camera_service.h"
#include "render/mesh_renderable.h"

// NOTE: currently only along V of the UV map
// TODO: currently scrolling with constant speed, find a way to speed up/slow down based on first timing group's current bpm
static MeshRenderable gen_mesh_tiled(Texture2D texture, float size_x, float size_y, float scale_x, float scale_y, bool enable_scroll) {
  Mesh mesh = GenMeshPlane(size_x, size_y, 1, 1);
  float textureAspect = (float)(texture.width * scale_x) / (float)(texture.height * scale_y);
  float track_tile_count_z = (size_y * textureAspect) / size_x;

  for (int i = 0; i < mesh.vertexCount; i++)
    mesh.texcoords[i * 2 + 1] *= track_tile_count_z;

  float *initial_texcoords = NULL;
  if (enable_scroll)
  {
    initial_texcoords = (float *)malloc(mesh.vertexCount * 2 * sizeof(float));
    memcpy(initial_texcoords, mesh.texcoords, mesh.vertexCount * 2 * sizeof(float));
  }
  UpdateMeshBuffer(mesh, 1, mesh.texcoords, mesh.vertexCount * 2 * sizeof(float), 0);

  Material material = LoadMaterialDefault();
  material.maps[MATERIAL_MAP_DIFFUSE].texture = texture;

  return (MeshRenderable){ .mesh = mesh, .material = material, .initial_texcoords = initial_texcoords };
}

TrackService track_service_init()
{
  TrackService service = {0};

  service.background_tex     = LoadTexture("resources/gameplay/DefaultBackgrounds/arccreate-blender2_base_light.jpg");
  service.track_tex          = LoadTexture("resources/gameplay/Track/TrackWhite.png");
  service.lane_div_tex       = LoadTexture("resources/gameplay/Track/TrackLaneDivider.png");
  service.critical_line_tex  = LoadTexture("resources/gameplay/CriticalLine/TrackCriticalLine.png");
  service.sky_input_line_tex = LoadTexture("resources/gameplay/CriticalLine/SkyInputLine.png");
  service.sky_label_tex      = LoadTexture("resources/gameplay/CriticalLine/SkyInputLabel.png");
  service.single_line_tex    = LoadTexture("resources/gameplay/SingleLine/SingleLineNone.png");

  GenTextureMipmaps(&service.background_tex);
  SetTextureFilter(service.background_tex, TEXTURE_FILTER_TRILINEAR);
  SetTextureFilter(service.lane_div_tex, TEXTURE_FILTER_BILINEAR);
  SetTextureWrap(service.track_tex, TEXTURE_WRAP_REPEAT);
  SetTextureWrap(service.single_line_tex, TEXTURE_WRAP_REPEAT);

  // --- Track (1 instance, tiled, scrolling) ---
  service.track = gen_mesh_tiled(service.track_tex, TRACK_SIZE_X, TRACK_SIZE_Y, 1.0f, 2.0f, true);
  renderable_set_transforms(&service.track, (Matrix[]){MatrixIdentity()}, 1);

  // --- Lane dividers (3 instances, same mesh/material) ---
  Mesh lane_div_mesh = GenMeshPlane(LANE_DIV_SIZE_X, LANE_DIV_SIZE_Y, 1, 1);
  UploadMesh(&lane_div_mesh, false);
  Material lane_div_material = LoadMaterialDefault();
  lane_div_material.maps[MATERIAL_MAP_DIFFUSE].texture = service.lane_div_tex;
  service.lane_div = (MeshRenderable){.mesh = lane_div_mesh, .material = lane_div_material};
  Matrix lane_div_scale = MatrixScale(0.5587685f, 12.43724f, 1.0f);
  renderable_set_transforms(
    &service.lane_div,
    (Matrix[]){
        MatrixMultiply(lane_div_scale, MatrixTranslate( 2.38f, 0.0f, 0.0f)),
        MatrixMultiply(lane_div_scale, MatrixTranslate(  0.0f, 0.0f, 0.0f)),
        MatrixMultiply(lane_div_scale, MatrixTranslate(-2.38f, 0.0f, 0.0f)),
    },
    3
  );

  // --- Critical lines (4 instances) ---
  Mesh critical_line_mesh = GenMeshPlane(CRITICAL_LINE_SIZE_X, CRITICAL_LINE_SIZE_Y, 1, 1);
  UploadMesh(&critical_line_mesh, false);
  Material critical_line_material = LoadMaterialDefault();
  critical_line_material.maps[MATERIAL_MAP_DIFFUSE].texture = service.critical_line_tex;
  service.critical_line = (MeshRenderable){.mesh = critical_line_mesh, .material = critical_line_material};
  renderable_set_transforms(
    &service.critical_line,
    (Matrix[]){
      MatrixTranslate( 3.565f, 0.0f, 0.0f),
      MatrixTranslate( 1.185f, 0.0f, 0.0f),
      MatrixTranslate(-1.185f, 0.0f, 0.0f),
      MatrixTranslate(-3.565f, 0.0f, 0.0f),
    },
    4
  );

  // --- Single line (2 instances - mirrored, tiled, scrolling) ---
  service.single_line = gen_mesh_tiled(service.single_line_tex, SINGLE_LINE_SIZE_X, SINGLE_LINE_SIZE_Y, 1.0f, 1.0f, true);
  Matrix single_line_rotate = MatrixMultiply(
      MatrixRotateX(90 * DEG2RAD),
      MatrixMultiply(MatrixMultiply(MatrixRotateZ(-90.0f * DEG2RAD),
                                    MatrixRotateX(145.0f * DEG2RAD)),
                     MatrixRotateY(90.0f * DEG2RAD)));
  Matrix single_line_rotate_flip = MatrixMultiply(
      MatrixRotateX(90 * DEG2RAD),
      MatrixMultiply(MatrixMultiply(MatrixRotateZ(-90.0f * DEG2RAD),
                                    MatrixRotateX(-145.0f * DEG2RAD)),
                     MatrixRotateY(90.0f * DEG2RAD)));
  renderable_set_transforms(
    &service.single_line,
    (Matrix[]){
      MatrixMultiply(single_line_rotate     , MatrixTranslate(-4.0f, 7.15f, 0.0f)),
      MatrixMultiply(single_line_rotate_flip, MatrixTranslate( 4.0f, 7.15f, 0.0f)),
    },
    2
  );

// --- Sky input line (1 instance, not tiled) ---
  Mesh sky_input_mesh = GenMeshPlane(SKY_INPUT_LINE_SIZE_X, SKY_INPUT_LINE_SIZE_Y, 1, 1);
  UploadMesh(&sky_input_mesh, false);
  Material sky_input_material = LoadMaterialDefault();
  sky_input_material.maps[MATERIAL_MAP_DIFFUSE].texture = service.sky_input_line_tex;
  service.sky_input_line = (MeshRenderable){.mesh = sky_input_mesh, .material = sky_input_material};
  renderable_set_transforms(&service.sky_input_line, (Matrix[]){MatrixIdentity()}, 1);

  Mesh sky_label_mesh = GenMeshPlane(SKY_LABEL_SIZE_X, SKY_LABEL_SIZE_Y, 1, 1);
  UploadMesh(&sky_label_mesh, false);

  RenderTexture2D sliced_sky_label = {0};
  {
    float pixels_per_unit = 150.0f;

    int render_tex_width  = (int)(SKY_LABEL_SIZE_X * pixels_per_unit / SKY_LABEL_SIZE_Y);
    int render_tex_height = (int)pixels_per_unit;
    sliced_sky_label = LoadRenderTexture(render_tex_width, render_tex_height);

    NPatchInfo sky_label_NPatch = {
      .source = (Rectangle){0, 0, (float)service.sky_label_tex.width, (float)service.sky_label_tex.height},
      .left   = 205,
      .top    = 0,
      .right  = 205,
      .bottom = 0,
      .layout = NPATCH_NINE_PATCH
    };
    BeginTextureMode(sliced_sky_label);
    ClearBackground(BLANK);
    DrawTextureNPatch(
        service.sky_label_tex, sky_label_NPatch,
        (Rectangle){0, 0, (float)render_tex_width, (float)render_tex_height},
        (Vector2){0, 0}, 0.0f, WHITE);
    EndTextureMode();
  }

  Material sky_label_material = LoadMaterialDefault();
  sky_label_material.maps[MATERIAL_MAP_DIFFUSE].texture = sliced_sky_label.texture;
  service.sky_label = (MeshRenderable){.mesh = sky_label_mesh, .material = sky_label_material};
  renderable_set_transforms(
    &service.sky_label,
    (Matrix[]){
      MatrixMultiply(MatrixRotateX(-90.0f * DEG2RAD), MatrixTranslate(0.0f, 0.17f, 0.0f))},
    1
  );
  return service;
}

void track_service_render_base_track(TrackService *track_service, ChartSettings *chart_settings, Camera3D camera)
{
  // Background
  float bg_scale = (float)GetScreenWidth() / (float)track_service->background_tex.width;
  DrawTextureEx(track_service->background_tex, (Vector2){0.0f, Lerp(-180.0f, 0.0f, get_aspect_ratio_adjustment()) * bg_scale}, 0.0f, bg_scale, WHITE);

  // Track-related
  BeginMode3D(camera);
    rlPushMatrix();
      rlScalef(1.7896f, 1.0f, 1.0f);

      rlDisableDepthTest();
      renderable_draw(&track_service->track);
      renderable_draw(&track_service->lane_div);
      renderable_draw(&track_service->critical_line);
      rlEnableDepthTest();
    rlPopMatrix();

    if (chart_settings->sl_type != SL_NONE)
    {
      BeginBlendMode(BLEND_ALPHA);
        rlDisableBackfaceCulling();
        rlDisableDepthMask();
        renderable_draw(&track_service->single_line);
        rlEnableDepthMask();
        rlEnableBackfaceCulling();
      EndBlendMode();
    }
  EndMode3D();
}

void track_service_render_sky_input(TrackService *track_service, Camera3D camera)
{
  // Sky input line - label
  BeginMode3D(camera);
    rlDisableDepthTest();
    rlPushMatrix();
      rlScalef(1.7896f, 1.0f, 1.0f);

      rlPushMatrix();
        rlTranslatef(0.0f, 5.5f, 0.0f);
        renderable_draw(&track_service->sky_input_line);

        rlDisableBackfaceCulling();
        renderable_draw(&track_service->sky_label);
        rlEnableBackfaceCulling();
      rlPopMatrix();
    rlPopMatrix();
    rlEnableDepthTest();
  EndMode3D();
}

void track_service_unload(TrackService *track_service)
{
  renderable_unload(&track_service->track);
  renderable_unload(&track_service->lane_div);
  renderable_unload(&track_service->critical_line);
  renderable_unload(&track_service->sky_input_line);
  renderable_unload(&track_service->sky_label);
  renderable_unload(&track_service->single_line);
  UnloadTexture(track_service->background_tex);
  UnloadTexture(track_service->track_tex);
  UnloadTexture(track_service->lane_div_tex);
  UnloadTexture(track_service->critical_line_tex);
  UnloadTexture(track_service->sky_input_line_tex);
  UnloadTexture(track_service->sky_label_tex);
  UnloadTexture(track_service->single_line_tex);
}
