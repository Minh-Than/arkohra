#ifndef WINDOW_SERVICES_H
#define WINDOW_SERVICES_H

#include "data/app_configs/app_config.h"
#include "data/fonts/fonts_service.h"
#include "window_inst.h"

typedef struct
{
  WindowInst command_palette;
  WindowInst project_setting;
} WindowService;

WindowService window_services_init(int glsl, AppConfigs *app_configs, CachedFontProbes *font_probes);
void window_services_handle_inputs(WindowService *window_service);
void window_services_window_check_oob(WindowService *window_service);
void windows_services_unload(WindowService *window_service);

#endif // WINDOW_SERVICES_H
