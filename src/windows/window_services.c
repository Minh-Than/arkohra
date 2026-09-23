#include <stdlib.h>
#include "raylib.h"
#include "constants.h"
#include "window_services.h"
#include "windows/command_palette/window.h"
#include "windows/project_setting/window.h"
#include "windows/window_inst.h"

WindowGroup window_services_init(int glsl, AppConfigs *app_configs)
{
  // Command Palette
  WindowInst command_palette    = window_inst_init(WINDOW_COMMAND_PALETTE);
  CmdPltData *cmd_plt_data      = (CmdPltData *)malloc(sizeof(CmdPltData));
  *cmd_plt_data                 = cmd_plt_init();
  command_palette.data          = cmd_plt_data;
  window_inst_ui_update(&command_palette,
                        (GetScreenWidth() - CMD_PLT_SIZE_X) / 2, CMD_PLT_Y, CMD_PLT_SIZE_X, CMD_PLT_SIZE_Y);

  // Command Palette
  WindowInst project_setting         = window_inst_init(WINDOW_PROJECT_SETTING);
  ProjSettingData *proj_setting_data = (ProjSettingData *)malloc(sizeof(ProjSettingData));
  *proj_setting_data                 = proj_setting_init(glsl, app_configs);
  project_setting.data               = proj_setting_data;

  int width = 528; int height = 448;
  window_inst_ui_update(&project_setting,
                        (GetScreenWidth() - width) / 2, (GetScreenHeight() - height) / 2, width, height);

  WindowGroup group = {
    .command_palette = command_palette,
    .project_setting = project_setting
  };

  return group;
}

void windows_services_render(WindowGroup *window_group, AppConfigs *app_configs, RenderContext *render_ctx)
{
  window_inst_draw(&window_group->command_palette, app_configs, render_ctx);
  window_inst_draw(&window_group->project_setting, app_configs, render_ctx);
}

void windows_services_unload(WindowGroup *window_group)
{
  window_inst_unload(&window_group->command_palette);

  ProjSettingData *proj_setting_data = (ProjSettingData *)window_group->project_setting.data;
  proj_setting_unload(proj_setting_data);
  window_inst_unload(&window_group->project_setting);
}
