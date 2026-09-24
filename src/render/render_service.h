# ifndef RENDER_SERVICE_H
# define RENDER_SERVICE_H

#include "raylib.h"
#include "data/chart_settings/chart_settings.h"
#include "gameplay/audio_service.h"
typedef struct
{
  Camera3D      camera;
  ChartSettings chart_settings;
  AudioClock    audio_clock;
} RenderContext;

#endif // RENDER_SERVICE_H
