#include <stdlib.h>
#include "raylib.h"
#include "constants.h"
#include "window_services.h"
#include "windows/command_palette/command_palette.h"
#include "windows/window_inst.h"

WindowGroup window_services_init()
{
  // Command Palette
  WindowInst command_palette    = window_inst_init(WINDOW_COMMAND_PALETTE);
  CmdPltData *cmd_plt_data      = (CmdPltData *)malloc(sizeof(CmdPltData));
  *cmd_plt_data                 = cmd_plt_init();
  command_palette.data          = cmd_plt_data;

  WindowGroup group = {
    .command_palette = command_palette
  };

  return group;
}

void windows_services_ui_update(WindowGroup (*window_group))
{
  window_inst_ui_update( &window_group->command_palette,
    (GetScreenWidth() - CMD_PLT_SIZE_X) / 2, CMD_PLT_Y, CMD_PLT_SIZE_X, CMD_PLT_SIZE_Y
  );
}

void windows_services_render(WindowGroup (*window_group))
{
  window_inst_draw(&window_group->command_palette);
}

void windows_services_unload(WindowGroup (*window_group))
{
  window_inst_unload(&window_group->command_palette);
}
