#ifndef CHART_READER_H
#define CHART_READER_H

#include <stdbool.h>
#include "data/custom_types/custom_types.h"
#include "raylib.h"
#include "render/layers/hold_tap_layer.h"
#include "render/layers/arc_layer.h"
#include "render/layers/arctap_layer.h"
#include "render/note_render_lists.h"
#include "render/render_service.h"

typedef struct {
  List timing_groups;
  NoteRenderLists render_lists;
  bool initialized;
} ChartReader;

ChartReader chart_reader_parse(char *file_path, RenderContext *render_ctx, Texture2D *arc_texture);
void chart_reader_render_notes(RenderContext *render_ctx, ChartReader* chart_reader,
                               HoldTapRenderer *hold_tap_renderer, ArcRenderer *arc_renderer, ArctapRenderer *arctap_renderer,
                               float current_ms);
void chart_reader_print(ChartReader *chart_reader);
void chart_reader_unload(ChartReader *chart_reader);

#endif // CHART_READER_H
