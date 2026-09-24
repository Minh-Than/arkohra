#include <string.h>
#include <stdlib.h>
#include "constants.h"
#include "raygui.h"
#include "raylib.h"
#include "../command_palette/window.h"

CmdPltData cmd_palette_init()
{
  CmdPltData data = { 0 };
  TextCopy(data.search_text, "");
  return data;
}

void cmd_palette_draw(WindowInst* window_inst, CmdPltData *data)
{
  if (!window_inst->is_visible) return;
  // Main panel
  Rectangle cmd_plt_rect = (Rectangle){ window_inst->x, window_inst->y, window_inst->width, window_inst->height };
  GuiPanel(cmd_plt_rect, NULL);

  // Search Text
  Rectangle search_text_rect = (Rectangle){
    cmd_plt_rect.x      + CMD_PLT_PADDING,
    cmd_plt_rect.y      + CMD_PLT_PADDING,
    window_inst->width  - CMD_PLT_PADDING * 2,
    window_inst->height - CMD_PLT_PADDING * 2
  };
  if (GuiTextBox(search_text_rect, data->search_text, 256, data->search_edit_node))
    data->search_edit_node = !data->search_edit_node;
}
