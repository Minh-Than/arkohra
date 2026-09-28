#ifndef NOTE_RENDER_LIST_H
#define NOTE_RENDER_LIST_H

#include <stdbool.h>
#include "data/chart_settings/chart_settings.h"
#include "data/custom_types/dynamic_list.h"
#include "data/keyframe/value_channel.h"

typedef struct {
  List beatline_render_list;
  List hold_render_list;
  List tap_render_list;
  List arc_render_list;
  List arccap_render_list;
  List arctap_render_list;

  ValueChannel enwidencamera_channel;
} NoteRenderLists;

void render_lists_init(NoteRenderLists *render_lists);
void render_lists_process(NoteRenderLists *render_lists, List *timing_groups, ChartSettings *chart_settings,
                          float current_ms, double *curr_fps, float *curr_bpms, double low_z_clip, double high_z_clip);
void render_lists_clear(NoteRenderLists *render_lists);
void render_lists_unload(NoteRenderLists *render_lists);

#endif // NOTE_RENDER_LIST_H
