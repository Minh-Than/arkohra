#ifndef GAMEPLAY_SCENE_OBJS_H
#define GAMEPLAY_SCENE_OBJS_H

#include "gameplay/chart_reader.h"
#include "raylib.h"
#include "render/layers/hold_tap_layer.h"
#include "render/layers/shadow_layer.h"
#include "render/layers/arc_layer.h"
#include "render/layers/arctap_layer.h"
#include "render/render_service.h"
#include "render/texture/texture_service.h"
#include "render/mesh_renderable.h"


typedef struct
{
  MeshRenderable track, lane_div, critical_line, sky_input_line, sky_label, single_line;

  HoldTapRenderer tap_hold_renderer;
  ArcRenderer     arc_renderer;
  ArctapRenderer  arctap_renderer;
  ShadowRenderer  shadow_renderer;
} PlayfieldObjs;

PlayfieldObjs playfield_objs_init(TextureGroup *texture_group);
void set_mesh_transforms(MeshRenderable *renderable, Matrix *transforms, int count);
void playfield_render(
  RenderContext *render_ctx,
  ChartReader *chart_reader,
  TextureGroup *texture_group,
  PlayfieldObjs *playfield_objs,
  float current_ms
);
void playfield_objs_unload(PlayfieldObjs *scene);

#endif // GAMEPLAY_SCENE_OBJS_H
