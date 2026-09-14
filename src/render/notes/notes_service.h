#ifndef NOTES_SERVICE_H
#define NOTES_SERVICE_H

#include "data/chart_settings/chart_settings.h"
#include "data/gameplay_events/arc_shader.h"
#include "gameplay/chart_reader.h"
#include "raylib.h"
#include "render/mesh_renderable.h"

typedef struct {
  Texture2D tap_tex, hold_tex, height_indicator_tex, arc_tex, arccap_tex, arctap_tex, arctap_shadow_tex;
  MeshRenderable tap, hold, arccap, arc_head, height_indicator, arctap, arctap_shadow;
  ArcShader arc_shader;
}NotesService;

NotesService notes_service_init(int glsl);
void notes_service_render(NotesService *notes_service, ChartSettings *chart_settings,
                          Camera3D camera, ChartReader *chart_reader, float current_ms);
void notes_service_unload(NotesService *notes_service);

#endif // NOTES_SERVICE_H
