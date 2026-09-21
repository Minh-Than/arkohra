#ifndef WINDOW_SERVICES_H
#define WINDOW_SERVICES_H

#include "window_inst.h"

typedef struct
{
  WindowInst command_palette;
  WindowInst project_setting;
} WindowGroup;

WindowGroup window_services_init(int glsl);
void windows_services_ui_update(WindowGroup (*window_group));
void windows_services_render(WindowGroup (*window_group));
void windows_services_unload(WindowGroup (*window_group));

#endif // WINDOW_SERVICES_H
