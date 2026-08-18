#include "playfield_services.h"
#include "constants.h"
#include "data/gameplay_events/arctap.h"
#include "data/gameplay_events/gameplay_events.h"
#include "gameplay/camera/camera_service.h"
#include "gameplay/chart_reader.h"
#include "raylib.h"
#include "raymath.h"
#include "render/mesh_renderable.h"
#include "rlgl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// NOTE: currently only along V of the UV map
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

void set_mesh_transforms(MeshRenderable *renderable, Matrix *transforms, int count) {
  renderable->transforms = (Matrix *)malloc(count * sizeof(Matrix));
  renderable->transform_count = count;
  memcpy(renderable->transforms, transforms, count * sizeof(Matrix));
}

PlayfieldObjs playfield_objs_init(TextureGroup *texture_group) {
  PlayfieldObjs objs = {0};

  // --- Track (1 instance, tiled, scrolling) ---
  objs.track = gen_mesh_tiled(texture_group->track, TRACK_SIZE_X, TRACK_SIZE_Y, 1.0f, 2.0f, true);
  set_mesh_transforms(&objs.track, (Matrix[]){MatrixIdentity()}, 1);

  // --- Lane dividers (3 instances, same mesh/material) ---
  Mesh lane_div_mesh = GenMeshPlane(LANE_DIV_SIZE_X, LANE_DIV_SIZE_Y, 1, 1);
  UploadMesh(&lane_div_mesh, false);
  Material lane_div_material = LoadMaterialDefault();
  lane_div_material.maps[MATERIAL_MAP_DIFFUSE].texture = texture_group->lane_div;
  objs.lane_div = (MeshRenderable){.mesh = lane_div_mesh, .material = lane_div_material};
  Matrix lane_div_scale = MatrixScale(0.5587685f, 12.43724f, 1.0f);
  set_mesh_transforms(
    &objs.lane_div,
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
  critical_line_material.maps[MATERIAL_MAP_DIFFUSE].texture = texture_group->critical_line;
  objs.critical_line = (MeshRenderable){.mesh = critical_line_mesh, .material = critical_line_material};
  set_mesh_transforms(
    &objs.critical_line,
    (Matrix[]){
      MatrixTranslate( 3.565f, 0.0f, 0.0f),
      MatrixTranslate( 1.185f, 0.0f, 0.0f),
      MatrixTranslate(-1.185f, 0.0f, 0.0f),
      MatrixTranslate(-3.565f, 0.0f, 0.0f),
    },
    4
  );

  // --- Single line (2 instances - mirrored, tiled, scrolling) ---
  objs.single_line = gen_mesh_tiled(texture_group->single_line, SINGLE_LINE_SIZE_X, SINGLE_LINE_SIZE_Y, 1.0f, 1.0f, true);
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
  set_mesh_transforms(
    &objs.single_line,
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
  sky_input_material.maps[MATERIAL_MAP_DIFFUSE].texture = texture_group->sky_input_line;
  objs.sky_input_line = (MeshRenderable){.mesh = sky_input_mesh, .material = sky_input_material};
  set_mesh_transforms(&objs.sky_input_line, (Matrix[]){MatrixIdentity()}, 1);

  Mesh sky_label_mesh = GenMeshPlane(SKY_LABEL_SIZE_X, SKY_LABEL_SIZE_Y, 1, 1);
  UploadMesh(&sky_label_mesh, false);

  RenderTexture2D sliced_sky_label = {0};
  {
    float pixels_per_unit = 150.0f;

    int render_tex_width  = (int)(SKY_LABEL_SIZE_X * pixels_per_unit / SKY_LABEL_SIZE_Y);
    int render_tex_height = (int)pixels_per_unit;
    sliced_sky_label = LoadRenderTexture(render_tex_width, render_tex_height);

    NPatchInfo sky_label_NPatch = {
      .source = (Rectangle){0, 0, (float)texture_group->sky_label.width, (float)texture_group->sky_label.height},
      .left   = 205,
      .top    = 0,
      .right  = 205,
      .bottom = 0,
      .layout = NPATCH_NINE_PATCH
    };
    BeginTextureMode(sliced_sky_label);
    ClearBackground(BLANK);
    DrawTextureNPatch(
        texture_group->sky_label, sky_label_NPatch,
        (Rectangle){0, 0, (float)render_tex_width, (float)render_tex_height},
        (Vector2){0, 0}, 0.0f, WHITE);
    EndTextureMode();
  }

  Material sky_label_material = LoadMaterialDefault();
  sky_label_material.maps[MATERIAL_MAP_DIFFUSE].texture = sliced_sky_label.texture;
  objs.sky_label = (MeshRenderable){.mesh = sky_label_mesh, .material = sky_label_material};
  set_mesh_transforms(
    &objs.sky_label,
    (Matrix[]){
      MatrixMultiply(MatrixRotateX(-90.0f * DEG2RAD), MatrixTranslate(0.0f, 0.17f, 0.0f))},
    1
  );

  objs.tap_hold_renderer = (HoldTapRenderer){
    .hold  = hold_load_mesh(&texture_group->hold),
    .tap   = tap_load_mesh(&texture_group->tap),
    .layer = LoadRenderTexture(GetScreenWidth() * SUPERSAMPLE_SCALE, GetScreenHeight() * SUPERSAMPLE_SCALE),
  };
  SetTextureFilter(objs.tap_hold_renderer.layer.texture, TEXTURE_FILTER_BILINEAR);

  objs.arc_renderer = (ArcRenderer){
    .layer = LoadRenderTexture(GetScreenWidth() * SUPERSAMPLE_SCALE, GetScreenHeight() * SUPERSAMPLE_SCALE),
  };
  SetTextureFilter(objs.arc_renderer.layer.texture, TEXTURE_FILTER_BILINEAR);

  objs.arctap_renderer = (ArctapRenderer){
    .arctap = arctap_load_mesh(&texture_group->arctap),
    .layer = LoadRenderTexture(GetScreenWidth() * SUPERSAMPLE_SCALE, GetScreenHeight() * SUPERSAMPLE_SCALE),
  };
  SetTextureFilter(objs.arctap_renderer.layer.texture, TEXTURE_FILTER_BILINEAR);

  objs.shadow_renderer = (ShadowRenderer){
    .arctap_shadow = arctap_shadow_load_mesh(&texture_group->arctap_shadow),
    .layer = LoadRenderTexture(GetScreenWidth() * SUPERSAMPLE_SCALE, GetScreenHeight() * SUPERSAMPLE_SCALE),
  };
  return objs;
}

void playfield_render(
  RenderContext *render_ctx,
  ChartReader *chart_reader,
  TextureGroup *texture_group,
  PlayfieldObjs *playfield_objs,
  float current_ms)
{
  // Background
  float bg_scale = (float)GetScreenWidth() / (float)texture_group->background.width;
  DrawTextureEx(texture_group->background, (Vector2){0.0f, Lerp(-180.0f, 0.0f, get_aspect_ratio_adjustment()) * bg_scale}, 0.0f, bg_scale, WHITE);

  // Tracks n shi
  BeginMode3D(render_ctx->camera);
    rlPushMatrix();
      rlScalef(1.7896f, 1.0f, 1.0f);

      rlDisableDepthTest();
      renderable_draw(&playfield_objs->track);
      renderable_draw(&playfield_objs->lane_div);
      renderable_draw(&playfield_objs->critical_line);
    rlPopMatrix();
  EndMode3D();

  // Gameplay notes
  chart_reader_render_notes(render_ctx, chart_reader,
                            &playfield_objs->tap_hold_renderer, &playfield_objs->shadow_renderer, &playfield_objs->arc_renderer, &playfield_objs->arctap_renderer,
                            current_ms);

  // Sky input line - label
  BeginMode3D(render_ctx->camera);
    rlDisableDepthTest();
    rlPushMatrix();
      rlScalef(1.7896f, 1.0f, 1.0f);

      rlPushMatrix();
        rlTranslatef(0.0f, 5.5f, 0.0f);
        renderable_draw(&playfield_objs->sky_input_line);
        rlDisableBackfaceCulling();
          renderable_draw(&playfield_objs->sky_label);
        rlEnableBackfaceCulling();
      rlPopMatrix();
    rlPopMatrix();

    if (IsTextureValid(texture_group->single_line))
    {
      BeginBlendMode(BLEND_ALPHA);
        rlDisableBackfaceCulling();
          renderable_draw(&playfield_objs->single_line);
        rlEnableBackfaceCulling();
      EndBlendMode();
    }
    rlEnableDepthTest();
  EndMode3D();
}

void playfield_objs_unload(PlayfieldObjs *scene) {
  renderable_unload(&scene->track);
  renderable_unload(&scene->lane_div);
  renderable_unload(&scene->critical_line);
  renderable_unload(&scene->sky_input_line);
  renderable_unload(&scene->sky_label);
  renderable_unload(&scene->single_line);

  renderable_unload(&scene->tap_hold_renderer.tap);
  renderable_unload(&scene->tap_hold_renderer.hold);
  UnloadRenderTexture(scene->tap_hold_renderer.layer);

  renderable_unload(&scene->shadow_renderer.arctap_shadow);
  UnloadRenderTexture(scene->shadow_renderer.layer);

  UnloadRenderTexture(scene->arc_renderer.layer);

  renderable_unload(&scene->arctap_renderer.arctap);
  UnloadRenderTexture(scene->arctap_renderer.layer);
}
