#ifndef HUD_SERVICES_H
#define HUD_SERVICES_H

#include "data/app_configs/app_config.h"
#include "data/chart_settings/chart_settings.h"
#include "render/texture/texture_service.h"

void hud_services_render(TextureGroup *texture_group, ChartSettings *chart_settings, FontServices *font_services);

#endif // HUD_SERVICES_H
