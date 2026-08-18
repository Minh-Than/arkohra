#ifndef WINDOW_SERVICES_H
#define WINDOW_SERVICES_H

#include "window_inst.h"

typedef struct
{
  WindowInst command_palette;
  // WindowInst general_setting;
} WindowGroup;

WindowGroup window_services_init();
void windows_services_ui_update(WindowGroup (*window_group));
void windows_services_render(WindowGroup (*window_group));
void windows_services_unload(WindowGroup (*window_group));

#endif // WINDOW_SERVICES_H
