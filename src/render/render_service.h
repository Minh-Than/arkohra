# ifndef RENDER_SERVICE_H
# define RENDER_SERVICE_H

#include "data/custom_types/custom_types.h"
#include "data/chart_settings/chart_settings.h"
#include "data/gameplay_events/arc_clip_shader.h"
#include "gameplay/audio_service.h"
#include "render/layers/arctap_layer.h"
#include "render/layers/hold_tap_layer.h"
#include "render/layers/shadow_layer.h"
#include "render/layers/arc_layer.h"
#include "raylib.h"
#include "render/texture/skin_side.h"
typedef struct
{
  Camera3D      camera;
  ChartSettings chart_settings;
  ArcClipShader arc_clip_shader;
  AudioClock    audio_clock;
} RenderContext;

void chart_reader_render_holds_taps(List *timing_groups, Camera *camera, HoldTapRenderer *hold_tap_renderer,
                                    float current_ms, float base_bpm, float scroll_speed, SkinSide side);
void chart_reader_render_shadows(List *timing_groups, Camera *camera, ShadowRenderer *shadow_renderer,
                                 float current_ms, float base_bpm, float scroll_speed);
void chart_reader_render_arcs(List *timing_groups, RenderContext *render_ctx, ArcRenderer *arc_renderer,
                              float current_ms, float base_bpm, float scroll_speed);
void chart_reader_render_arctaps(List *timing_groups, Camera *camera, ArctapRenderer *arctap_renderer,
                                 float current_ms, float base_bpm, float scroll_speed);

#endif // RENDER_SERVICE_H
