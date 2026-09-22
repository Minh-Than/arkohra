#include "render_service.h"
#include "gameplay/audio_service.h"

void render_scenes(RenderContext *render_ctx, ChartReader *chart_reader)
{
  float final_current_ms = audio_clock_get_time_ms(&render_ctx->audio_clock) - render_ctx->chart_settings.audio_offset;
  track_service_render_base_track(&render_ctx->track_service, &render_ctx->chart_settings, render_ctx->camera);
  if (chart_reader->initialized)
    notes_service_render(&render_ctx->notes_service, &render_ctx->chart_settings,
                          render_ctx->camera, chart_reader, final_current_ms);
  track_service_render_sky_input(&render_ctx->track_service, render_ctx->camera);
  hud_services_render(&render_ctx->hud_service, &render_ctx->chart_settings, &render_ctx->audio_clock, final_current_ms);
}

void render_unload(RenderContext *render_ctx)   
{
  hud_service_unload  (&render_ctx->hud_service);
  track_service_unload(&render_ctx->track_service);
  notes_service_unload(&render_ctx->notes_service);
}
