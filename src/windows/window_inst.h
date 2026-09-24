#ifndef WINDOW_INST_H
#define WINDOW_INST_H

#include <stdbool.h>

typedef enum
{
  WINDOW_COMMAND_PALETTE,
  WINDOW_PROJECT_SETTING
} WindowType;

typedef struct
{
  void *     data;
  float      x, y, width, height;
  WindowType type;
  bool       is_visible;
} WindowInst;

WindowInst window_inst_init(WindowType type);
void window_inst_ui_update(WindowInst *window_inst, int x, int y, int width, int height);
void window_inst_open(WindowInst *window_inst);
void window_inst_close(WindowInst *window_inst);
void window_inst_toggle(WindowInst *window_inst);
void window_inst_unload(WindowInst *window_inst);

#endif // WINDOW_INST_H
