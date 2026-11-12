#include "render_service.h"
#include "gameplay/camera/camera_service.h"

RenderContext render_context_init(AppConfigs *app_configs)
{
  RenderContext render_ctx = {
    .camera         = camera_init_playfield(),
    .chart_settings = chart_settings_init(app_configs),
    .audio_clock    = { 0 },
  };

  return render_ctx;
}
