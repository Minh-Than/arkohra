#ifndef HUD_SERVICES_H
#define HUD_SERVICES_H

#include "data/app_configs/app_config.h"
#include "render/render_service.h"
#include "render/texture/texture_service.h"

void hud_services_render(TextureGroup *texture_group, RenderContext *render_ctx, FontServices *font_services, float current_ms);

#endif // HUD_SERVICES_H
