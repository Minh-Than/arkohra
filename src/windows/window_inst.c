#include <math.h>
#include <stdlib.h>
#include "raygui.h"
#include "windows/command_palette/window.h"
#include "window_inst.h"

WindowInst window_inst_init(WindowType type, int x, int y, int width, int height)
{
  return (WindowInst){
    .data       = NULL,
    .x          = x,
    .y          = y,
    .width      = width,
    .height     = height,
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

void window_inst_drag_resize(WindowInst *window_inst, int min_width, int min_height)
{
  Rectangle rect = (Rectangle){ window_inst->x, window_inst->y,
                                    window_inst->width, window_inst->height };
  static bool resizing = false;
  Rectangle handle  = (Rectangle){ rect.x + rect.width  - 10,
                                   rect.y + rect.height - 10,
                                   20, 20 };
  if (resizing)
  {
    window_inst->width  = fmaxf(min_width, fminf(GetScreenWidth() , GetMouseX() - window_inst->x));
    window_inst->height = fmaxf(min_height, fminf(GetScreenHeight(), GetMouseY() - window_inst->y));
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) resizing = false;
  }
  else if (CheckCollisionPointRec(GetMousePosition(), handle) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    resizing = true;

  static bool dragging = false;
  static Vector2 dragging_pos = { 0, 0 };
  Rectangle top_bar = (Rectangle){ rect.x, rect.y, rect.width, 24 };
  if(dragging)
  {
    window_inst->x = GetMouseX() - dragging_pos.x;
    window_inst->y = GetMouseY() - dragging_pos.y;
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) dragging = false;
  }
  else if (CheckCollisionPointRec(GetMousePosition(), top_bar) &&
           IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
  {
    dragging = true;
    dragging_pos.x = GetMouseX() - window_inst->x;
    dragging_pos.y = GetMouseY() - window_inst->y;
  }


  DrawTriangle((Vector2){ handle.x + 8, handle.y + 8 },
               (Vector2){ handle.x + 8, handle.y - 1 },
               (Vector2){ handle.x - 2, handle.y + 8 }, GRAY);
}

void window_inst_unload(WindowInst *window_inst)
{
  free(window_inst->data);
  window_inst->data = NULL;
}
