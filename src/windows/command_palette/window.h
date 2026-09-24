#ifndef WIN_COMMAND_PALETTE_H
#define WIN_COMMAND_PALETTE_H

#include "windows/window_inst.h"

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

CmdPltData cmd_palette_init();
void cmd_palette_draw(WindowInst* window_inst, CmdPltData *data);

#endif // WIN_COMMAND_PALETTE_H
