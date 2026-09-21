#include <stdlib.h>
#include "raygui.h"
#include "windows/command_palette/window.h"
#include "windows/project_setting/window.h"
#include "window_inst.h"

WindowInst window_inst_init(WindowType type)
{
  return (WindowInst){
    .data       = NULL,
    .x          = 0,
    .y          = 0,
    .width      = 0,
    .height     = 0,
    .type       = type,
    .is_visible = false
  };
}

void window_inst_ui_update(WindowInst *window_inst, int x, int y, int width, int height)
{
  window_inst->x        = x;
  window_inst->y        = y;
  window_inst->width    = width;
  window_inst->height   = height;
}

void window_inst_open(WindowInst *window_inst)
{
  window_inst->is_visible = true;
  switch (window_inst->type) {
    case WINDOW_COMMAND_PALETTE: {
      CmdPltData *data          = (CmdPltData *)window_inst->data;
      data->search_edit_node    = true;
      break;
    }
    case WINDOW_PROJECT_SETTING: break;
  }
}

void window_inst_close(WindowInst *window_inst)
{
  window_inst->is_visible = false;
  switch (window_inst->type) {
    case WINDOW_COMMAND_PALETTE: {
      CmdPltData *data          = (CmdPltData *)window_inst->data;
      data->search_edit_node    = false;
      break;
    }
    case WINDOW_PROJECT_SETTING: break;
  }
}

void window_inst_toggle(WindowInst *window_inst)
{
  if (window_inst->is_visible) window_inst_close(window_inst);
  else window_inst_open(window_inst);
}

void window_inst_draw(WindowInst *window_inst)
{
  if (!window_inst->is_visible) return;

  switch (window_inst->type)
  {
    case WINDOW_COMMAND_PALETTE: cmd_plt_draw(window_inst, (CmdPltData *)window_inst->data); break;
    case WINDOW_PROJECT_SETTING: proj_setting_draw(window_inst, (ProjSettingData *)window_inst->data); break;
  }
}

void window_inst_unload(WindowInst *window_inst)
{
  free(window_inst->data);
  window_inst->data = NULL;
}
