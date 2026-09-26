#ifndef NOTES_SERVICE_H
#define NOTES_SERVICE_H

#include "data/gameplay_events/arc_shader.h"
#include "gameplay/chart_reader.h"
#include "raylib.h"
#include "render/mesh_renderable.h"
#include "render/render_service.h"
#include "render/track/track_service.h"

typedef struct {
  Texture2D tap_tex, hold_tex, height_indicator_tex, arc_tex, arccap_tex, arctap_tex, arctap_shadow_tex;
  MeshRenderable tap, hold, arccap, arc_head, height_indicator, arctap, arctap_shadow;
  ArcShader arc_shader;
}NotesService;

NotesService notes_service_init(int glsl);
void notes_service_render(NotesService *notes_service, TrackService *track_service, RenderContext *render_ctx, ChartReader *chart_reader, bool colorblind);
void notes_service_apply_chart(NotesService *notes_service, ChartSettings *chart_settings);
void notes_service_unload(NotesService *notes_service);

#endif // NOTES_SERVICE_H
