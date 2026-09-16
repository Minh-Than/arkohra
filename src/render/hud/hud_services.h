#ifndef HUD_SERVICES_H
#define HUD_SERVICES_H

#include "data/chart_settings/chart_settings.h"
#include "data/fonts/fonts_service.h"
#include "gameplay/audio_service.h"
#include "raylib.h"

typedef struct {
  Shader sdf_shader;
  Font saira_medium, saira_regular;
  Texture2D pause_button, info_panel, jacket_bg, jacket_img, jacket_diff, progress_glow;
  FontChain font_chain;
} HudService;

HudService hud_service_init(int glsl);
void hud_services_render(HudService *hud_service, ChartSettings *chart_settings, AudioClock *audio_clock, float current_ms);
void hud_service_unload(HudService *hud_service);

#endif // HUD_SERVICES_H
