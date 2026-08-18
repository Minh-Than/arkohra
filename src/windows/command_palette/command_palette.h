#ifndef COMMAND_PALETTE_H
#define COMMAND_PALETTE_H

#include "windows/window_inst.h"
// #include "data/custom_types/custom_types.h"

typedef struct
{
  const char* label;
  void (* action)(void *);
  void *context;
} CommandOpt;

typedef struct
{
  // List commands;
  char search_text[256];
  bool search_edit_node;
} CmdPltData;

CmdPltData cmd_plt_init();
void cmd_plt_draw(WindowInst* window_inst, CmdPltData *data);

#endif // COMMAND_PALETTE_H
