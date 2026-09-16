# ifndef RENDER_SERVICE_H
# define RENDER_SERVICE_H

#include "raylib.h"
#include "data/chart_settings/chart_settings.h"
#include "gameplay/audio_service.h"
#include "gameplay/chart_reader.h"
#include "render/hud/hud_services.h"
#include "render/notes/notes_service.h"
#include "render/track/track_service.h"
typedef struct
{
  Camera3D      camera;
  ChartSettings chart_settings;
  AudioClock    audio_clock;
  TrackService  track_service;
  NotesService  notes_service;
  HudService    hud_service;
} RenderContext;

void render_scenes(RenderContext *render_ctx, ChartReader *chart_reader, float current_ms);
void render_unload(RenderContext *render_ctx);

#endif // RENDER_SERVICE_H
