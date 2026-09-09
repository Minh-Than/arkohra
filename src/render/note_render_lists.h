#ifndef NOTE_RENDER_LIST_H
#define NOTE_RENDER_LIST_H

#include <stdbool.h>
#include "data/custom_types/dynamic_list.h"

typedef struct {
  List hold_render_list;
  List tap_render_list;
  List arc_render_list;
  List arccap_render_list;
  List arctap_render_list;
} NoteRenderLists;

void render_lists_initialize(NoteRenderLists *render_lists);
void render_lists_clear(NoteRenderLists *render_lists);
void render_lists_unload(NoteRenderLists *render_lists);

#endif // NOTE_RENDER_LIST_H
