#ifndef BEATLINE_H
#define BEATLINE_H

#include "raylib.h"
#include "data/chart_timing_groups/chart_timing_group.h"
#include "data/custom_types/dynamic_list.h"

typedef struct {
  Color color;
  double fp;
  float thickness;
  int timing, timing_group;
} BeatLine;

List beatline_generate(ChartTimingGroup *tg, int audio_length_ms, int audio_offset, Color color, float thickness);
int beatline_compare_fp_asc(const void *a, const void *b);
void beatline_print(const void* elem);

#endif // BEATLINE_H
