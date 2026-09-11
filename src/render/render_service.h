# ifndef RENDER_SERVICE_H
# define RENDER_SERVICE_H

#include "raylib.h"
#include "data/chart_settings/chart_settings.h"
#include "data/custom_types/dynamic_list.h"
#include "data/gameplay_events/arc_shader.h"
#include "gameplay/audio_service.h"
#include "render/layers/arctap_layer.h"
#include "render/layers/hold_tap_layer.h"
#include "render/layers/arc_layer.h"
#include "render/note_render_lists.h"
typedef struct
{
  Camera3D      camera;
  ChartSettings chart_settings;
  ArcShader     arc_shader;
  AudioClock    audio_clock;
} RenderContext;

void render_holds_taps(List *timing_groups, NoteRenderLists *note_render_lists, RenderContext *render_ctx, HoldTapRenderer *hold_tap_renderer,
                       float current_ms, float base_bpm, float scroll_speed, double *curr_fps);
void render_arcs_and_shadows(List *timing_groups, NoteRenderLists *note_render_lists, RenderContext *render_ctx, ArcRenderer *arc_renderer,
                             float current_ms, float base_bpm, float scroll_speed, double *curr_fps, float *curr_bpms);
void render_arctaps(List *timing_groups, NoteRenderLists *note_render_lists, RenderContext *render_ctx, ArctapRenderer *arctap_renderer,
                    float current_ms, float base_bpm, float scroll_speed, double *curr_fps, float *curr_bpms);

#endif // RENDER_SERVICE_H
