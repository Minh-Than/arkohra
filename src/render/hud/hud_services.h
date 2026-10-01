#ifndef HUD_SERVICES_H
#define HUD_SERVICES_H

#include "data/app_configs/app_config.h"
#include "data/chart_settings/chart_settings.h"
#include "data/custom_types/dynamic_list.h"
#include "data/fonts/fonts_service.h"
#include "raylib.h"
#include "render/render_service.h"

typedef struct {
  Shader sdf_shader;
  Font saira_regular, saira_medium;
  Texture2D pause_button, info_panel, jacket_bg, jacket_img, jacket_diff, progress_glow;
  List font_with_fallback;

  AspectRatio playfield_ratio;
  char jacket_path[MAXPATHLEN];
} HudService;

HudService hud_service_init(int glsl, AppConfigs *app_configs, CachedFontProbes *font_probes);
void hud_services_render(HudService *hud_service, RenderContext *render_ctx);
void hud_service_apply_chart(HudService *hud_service, ChartSettings *chart_settings, CachedFontProbes *font_probes);
void hud_service_unload(HudService *hud_service);

#endif // HUD_SERVICES_H
