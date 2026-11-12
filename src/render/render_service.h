# ifndef RENDER_SERVICE_H
# define RENDER_SERVICE_H

#include "data/app_configs/app_config.h"
#include "raylib.h"
#include "data/chart_settings/chart_settings.h"
#include "gameplay/audio_service.h"
typedef struct
{
  Camera3D      camera;
  ChartSettings chart_settings;
  AudioClock    audio_clock;
} RenderContext;

RenderContext render_context_init(AppConfigs *app_configs);

#endif // RENDER_SERVICE_H
