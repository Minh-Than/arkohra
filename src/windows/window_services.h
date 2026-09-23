#ifndef WINDOW_SERVICES_H
#define WINDOW_SERVICES_H

#include "data/app_configs/app_config.h"
#include "render/render_service.h"
#include "window_inst.h"

typedef struct
{
  WindowInst command_palette;
  WindowInst project_setting;
} WindowGroup;

WindowGroup window_services_init(int glsl, AppConfigs *app_configs);
void windows_services_render(WindowGroup *window_group, AppConfigs *app_configs, RenderContext *render_ctx);
void windows_services_unload(WindowGroup *window_group);

#endif // WINDOW_SERVICES_H
