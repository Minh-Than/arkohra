#ifndef CHART_READER_H
#define CHART_READER_H

#include <stdbool.h>
#include "data/chart_settings/chart_settings.h"
#include "gameplay/audio_service.h"
#include "render/note_render_lists.h"

typedef struct {
  List timing_groups;
  NoteRenderLists render_lists;
  double low_z_clip, high_z_clip;
  bool initialized;
  int tap_counter, hold_counter, arc_counter, arctap_counter;
} ChartReader;

ChartReader chart_reader_parse(ChartSettings *chart_settings, AudioClock *audio_clock);
void chart_reader_print(ChartReader *chart_reader);
void chart_reader_unload(ChartReader *chart_reader);

#endif // CHART_READER_H
