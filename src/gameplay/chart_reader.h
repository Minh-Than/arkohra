#ifndef CHART_READER_H
#define CHART_READER_H

#include <stdbool.h>
#include "data/chart_settings/chart_settings.h"
#include "gameplay/audio_service.h"
#include "raylib.h"
#include "render/note_render_lists.h"

typedef struct {
  List timing_groups;
  NoteRenderLists render_lists;
  float low_z_clip, high_z_clip;
  bool initialized;
} ChartReader;

ChartReader chart_reader_parse(char *file_path, ChartSettings *chart_settings, AudioClock *audio_clock, Texture2D *arc_texture, Shader *arc_shader);
void chart_reader_print(ChartReader *chart_reader);
void chart_reader_unload(ChartReader *chart_reader);
void process_note_render_lists(ChartReader *chart_reader, ChartSettings *chart_settings, float current_ms, double *curr_fps, float *curr_bpms);

#endif // CHART_READER_H
