#ifndef TRACK_SERVICE_H
#define TRACK_SERVICE_H

#include "raylib.h"
#include "render/mesh_renderable.h"
#include "render/note_render_lists.h"
#include "render/render_service.h"

typedef struct {
  Texture2D background_tex, track_tex, lane_div_tex, critical_line_tex, sky_input_line_tex, sky_label_tex, single_line_tex, extra_lane_tex;
  MeshRenderable track, lane_div, critical_line, sky_input_line, sky_label, single_line;
  MeshRenderable extra_critical_line, extra_lane, extra_lane_div, extra_lane_edge;
  Shader track_shader, single_line_shader;
  int track_scrollOffset_loc;
  int track_enwidenlanesValue_loc;
  int single_line_scrollOffset_loc;

  char background_path[MAXPATHLEN];
  SkinSide skin_track, skin_side;
  SingleLineType sl_type;
} TrackService;

TrackService track_service_init(int glsl);
void set_mesh_transforms(MeshRenderable *renderable, Matrix *transforms, int count);
void track_service_render_base_track(TrackService *track_service, RenderContext *render_ctx, NoteRenderLists *note_render_lists);
void track_service_render_sky_input(TrackService *track_service, RenderContext *render_ctx, NoteRenderLists *note_render_lists);
void track_service_apply_chart(TrackService *track_service, ChartSettings *chart_settings);
void track_service_unload(TrackService *track_service);

#endif // TRACK_SERVICE_H
