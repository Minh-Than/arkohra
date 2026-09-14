#ifndef TRACK_SERVICE_H
#define TRACK_SERVICE_H

#include "raylib.h"
#include "data/chart_settings/chart_settings.h"
#include "render/mesh_renderable.h"

typedef struct {
  Texture2D background_tex, track_tex, lane_div_tex, critical_line_tex, sky_input_line_tex, sky_label_tex, single_line_tex;
  MeshRenderable track, lane_div, critical_line, sky_input_line, sky_label, single_line;
} TrackService;

TrackService track_service_init();
void set_mesh_transforms(MeshRenderable *renderable, Matrix *transforms, int count);
void track_service_render_base_track(TrackService *track_service, ChartSettings *chart_settings, Camera3D camera);
void track_service_render_sky_input(TrackService *track_service, Camera3D camera);
void track_service_unload(TrackService *track_service);

#endif // TRACK_SERVICE_H
