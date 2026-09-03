#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "raylib.h"
#include "rlgl.h"
#include "chart_reader.h"
#include "data/chart_timing_groups/chart_timing_group.h"
#include "data/custom_types/custom_types.h"
#include "data/gameplay_events/arc.h"
#include "data/gameplay_events/gameplay_events.h"
#include "gameplay/arc_formula.h"
#include "render/render_service.h"

static void chart_reader_rebuild_arctaps(ChartTimingGroup *tg)
{
  tg->arctaps.size = 0;

  // Pass 1: rebuild tg->arctaps from arc->arctaps
  for (int j = 0; j < tg->arcs.size; j++)
  {
    Arc *arc = (Arc *)list_get(&tg->arcs, j);
    for (int k = 0; k < arc->arctaps.size; k++)
    {
      ArcTap *arctap = (ArcTap *)list_get(&arc->arctaps, k);
      arctap->arc = arc;
      arctap->fp  = get_floor_position(&tg->timing_events, arctap->timing);
      list_push(&tg->arctaps, arctap);
    }
  }

  // Pass 2: set arc pointers on tg->arctaps copies
  //         (tg->arcs buffer is frozen — no more pushes to it)
  int idx = 0;
  for (int j = 0; j < tg->arcs.size; j++)
  {
    Arc *arc = (Arc *)list_get(&tg->arcs, j);
    for (int k = 0; k < arc->arctaps.size; k++)
    {
      if (idx < tg->arctaps.size)
      {
        ArcTap *tg_at = (ArcTap *)list_get(&tg->arctaps, idx);
        tg_at->arc = arc;
        idx++;
      }
    }
  }
}

static bool parse_aff_header(char *line, ChartSettings *chart_settings)
{
  bool end_of_header = strcmp(line, "-") == 0;

  if (strstr(line, (const char*)"AudioOffset") != NULL)
  {
    if(strncmp(line, "AudioOffset:", 12) == 0)
    {
      int offset;
      int matched = sscanf(line, "AudioOffset:%d", &offset);
      if (matched != 1) return end_of_header;
      chart_settings->audio_offset = offset;
    }
  }

  return end_of_header;
}

static int parse_arctaps(const char *line, List *out)
{
  const char *lb = strchr(line, '['); if (!lb) return 0;
  const char *rb = strchr(lb  , ']'); if (!rb) return 0;

  int count = 0;
  const char *p = lb + 1;

  while (p < rb)
  {
    const char *tap = strstr(p, "arctap(");
    if (!tap || tap >= rb) break;

    p = tap + 7; // Start at character after "arctap("
    char *end;
    int val = strtol(p, &end, 10);

    // No number found, skip ahead
    if (end == p) { p++; continue; }

    list_push(out, &val);
    p = end; // end points past the number, at ')'
  }

  return count;
}

static void parse_aff_lines(char *line, ChartReader *chart_reader, int *tg_count, int *current_tg)
{
  ChartTimingGroup *tg = (ChartTimingGroup *)list_get(&chart_reader->timing_groups, *current_tg);
  RawEventType type = determine_type(line);
  switch (type)
  {
    case TIMING_GROUP:
      {
        ChartTimingGroup init_tg = timing_group_init();
        list_push(&chart_reader->timing_groups, &init_tg);
        *tg_count += 1;
        *current_tg = *tg_count - 1;
        break;
      }
    case TIMING_EVENT:
      {
        int timing;
        float bpm, divisor;
        int matched = sscanf(line, "timing(%d,%f,%f);", &timing, &bpm, &divisor);
        if (matched == 3)
        {
          TimingEvent t_event = { .fp = 0, .bpm = bpm, .divisor = divisor, .timing = timing, .timing_group = *current_tg };
          list_push(&tg->timing_events, &t_event);
        }
        break;
      }
    case TAP:
      {
        int timing;
        float lane;
        int matched = sscanf(line, "(%d,%f);", &timing, &lane);
        if (matched == 2)
        {
          List connector_x; list_init(&connector_x, sizeof(float));
          List connector_y; list_init(&connector_y, sizeof(float));
          Tap tap = (Tap) { .connector_x = connector_x, .connector_y = connector_y, .lane = lane, .timing = timing, .timing_group = *current_tg };
          list_push(&tg->taps, &tap);
        }
        break;
      }
    case HOLD:
      {
        int start_timing, end_timing;
        float lane;
        int matched = sscanf(line, "hold(%d,%d,%f);", &start_timing, &end_timing, &lane);
        if (matched == 3)
        {
          Hold hold = {
            .lane = lane,
            .start_timing = start_timing,
            .end_timing   = end_timing,
            .timing_group = *current_tg,
          };
          list_push(&tg->holds, &hold);
        }
        break;
      }
    case ARC:
      {
        int start_timing, end_timing;
        float x1, y1, x2, y2;
        char arc_type[8];
        int color;
        char sfx[256], is_void[8];
        int matched = sscanf(line, "arc(%d,%d,%f,%f,%7[^,],%f,%f,%d,%255[^,],%7[^)])",
                             &start_timing, &end_timing, &x1, &x2, arc_type, &y1, &y2, &color, sfx, is_void);
        if (matched == 10)
        {
          List arctaps; list_init(&arctaps, sizeof(ArcTap));
          Arc arc = {
            .arctaps      = arctaps,
            .x1 = x1, .y1 = y1,
            .x2 = x2, .y2 = y2,
            .arc_res      = 1.0f,
            .start_timing = start_timing,
            .end_timing   = end_timing,
            .timing_group = *current_tg,
            .color        = color,
            .type         = arctype_get_by_string(arc_type),
            .is_void      = strncmp(is_void, "true", 4) == 0 ? true : false,
            .is_head      = true
          };
          strncpy(arc.sfx, sfx, sizeof(arc.sfx) - 1);

          List arctap_timings; list_init(&arctap_timings, sizeof(int));
          parse_arctaps(line, &arctap_timings);
          for (int i = 0; i < arctap_timings.size; i++)
          {
            int *timing = (int *)list_get(&arctap_timings, i);
            ArcTap arctap = { .arc = NULL, .width = 1.0f, .timing = *timing, .timing_group = *current_tg };
            list_push(&arc.arctaps, &arctap);
          }
          list_free(&arctap_timings);

          list_push(&tg->arcs, &arc);
        }
      }
      break;
    default: break;
  }
}

static void parse_post_process(RenderContext *render_ctx, ChartReader *chart_reader, Texture2D *arc_texture)
{
  for(int i = 0; i < chart_reader->timing_groups.size; i++)
  {
    ChartTimingGroup *tg = (ChartTimingGroup *)list_get(&chart_reader->timing_groups, i);
    list_sort_by(&tg->timing_events, timing_event_compare_timing_asc);
    recalculate_floor_position(tg);

    list_sort_by(&tg->arcs, arc_compare_start_timing_asc);
    chart_reader_rebuild_arctaps(tg);

    // Pre-calculate the notes's floor position
    // Arctaps' floor position is already calculated in `chart_reader_rebuild_arctaps`
    for (int j = 0; j < tg->holds.size; j++)
    {
      Hold *hold = (Hold *)list_get(&tg->holds, j);
      hold->start_fp = get_floor_position(&tg->timing_events, hold->start_timing);
      hold->end_fp   = get_floor_position(&tg->timing_events, hold->end_timing);
    }

    for (int j = 0; j < tg->taps.size; j++)
    {
      Tap *tap = (Tap *)list_get(&tg->taps, j);
      tap->fp  = get_floor_position(&tg->timing_events, tap->timing);

      TapFP tap_fp = { .tap = tap, .fp = tap->fp };
      list_push(&tg->tap_fps, &tap_fp);

      for (int k = 0; k < tg->arctaps.size; k++)
      {
        ArcTap *arctap = (ArcTap *)list_get(&tg->arctaps, k);
        if (roundf(fabsf((float)(arctap->timing - tap->timing))) < 2)
        {
          float x = arc_world_x_at(arctap->timing, arctap->arc);
          float y = arc_world_y_at(arctap->timing, arctap->arc);
          list_push(&tap->connector_x, &x);
          list_push(&tap->connector_y, &y);
        }
      }
    }
    list_sort_by(&tg->tap_fps, tapfp_compare_fp_asc);

    for (int j = 0; j < tg->arcs.size; j++)
    {
      Arc *arc = (Arc *)list_get(&tg->arcs, j);
      arc->start_fp = get_floor_position(&tg->timing_events, arc->start_timing);
      arc->end_fp   = get_floor_position(&tg->timing_events, arc->end_timing);

      if (!arc->is_void)
      {
        Arc low_target  = { .start_timing = arc->end_timing - 1 };
        Arc high_target = { .start_timing = arc->end_timing + 1 };
        int start_idx = bisect_left(&tg->arcs, &low_target, arc_compare_start_timing_asc);
        int end_idx = bisect_right(&tg->arcs, &high_target, arc_compare_start_timing_asc);
        for (int k = start_idx; k < end_idx; k++)
        {
          Arc *connected_arc = (Arc *)list_get(&tg->arcs, k);
          if (fabsf(connected_arc->x1 - arc->x2) < 1e-6 && fabsf(connected_arc->y1 - arc->y2) < 1e-6)
            connected_arc->is_head = false;
        }
      }

      int arc_duration      = arc->end_timing - arc->start_timing;
      float segment_length  = calculate_arc_segment_length(arc_duration, arc->arc_res);
      int segment_count     = (int)ceilf(arc_duration / segment_length);
      segment_count         = (int)fmax(segment_count, 1);

      for (int k = 0; k < segment_count; k++)
      {
        ArcSegment arc_segment = { .arc = arc };
        list_push(&tg->arc_segments, &arc_segment);
      }
      arc_segment_generate_mesh(&tg->arc_segments, arc, arc_texture, &tg->timing_events, render_ctx);
      shadow_segment_generate_mesh(&tg->arc_segments, arc, &tg->timing_events, render_ctx);
    };
    list_sort_by(&tg->arc_segments, arc_segment_compare_start_fp_asc);

    for (int j = 0; j < tg->arctaps.size; j++)
    {
      ArcTap *arctap = (ArcTap *)list_get(&tg->arctaps, j);
      ArcTapFP arctap_fp = { .arctap = arctap, .fp = arctap->fp };
      list_push(&tg->arctap_fps, &arctap_fp);
    }
    list_sort_by(&tg->arctap_fps, arctapfp_compare_fp_asc);
  }
}

ChartReader chart_reader_parse(char *file_path, RenderContext *render_ctx, Texture2D *arc_texture)
{
  ChartReader chart_reader = { 0 };

  char *aff_data = LoadFileText(file_path);
  if (aff_data == NULL) return chart_reader;

  List tgs; list_init(&tgs, sizeof(ChartTimingGroup));
  ChartTimingGroup init_tg = timing_group_init();
  list_push(&tgs, &init_tg);
  chart_reader.timing_groups = tgs;

  int line_count = 0;
  char **lines = LoadTextLines(aff_data, &line_count);

  bool is_header = true;
  int tg_count   = 1;
  int current_tg = 0;
  for (int i = 0; i < line_count; i++)
  {
    char *line = lines[i];
    line = trim_whitespace(line);

    if (is_header)
    {
      if (parse_aff_header(line, &render_ctx->chart_settings)) is_header = false;
      continue;
    }

    if(strcmp(line, "};")  == 0) { current_tg = 0; continue; }

    parse_aff_lines(line, &chart_reader, &tg_count, &current_tg);

  }
  UnloadFileText(aff_data);

  parse_post_process(render_ctx, &chart_reader, arc_texture);
  chart_reader.initialized = true;
  return chart_reader;
}

void chart_reader_render_notes(RenderContext *render_ctx, ChartReader* chart_reader,
                               HoldTapRenderer *hold_tap_renderer, ArcRenderer *arc_renderer, ArctapRenderer *arctap_renderer,
                               float current_ms)
{
  if (!chart_reader->initialized) return;
  float base_bpm     = render_ctx->chart_settings.base_bpm;
  float scroll_speed = render_ctx->chart_settings.scroll_speed;
  render_holds_taps      (&chart_reader->timing_groups, render_ctx, hold_tap_renderer, current_ms, base_bpm, scroll_speed);
  render_arcs_and_shadows(&chart_reader->timing_groups, render_ctx, arc_renderer     , current_ms, base_bpm, scroll_speed);
  render_arctaps         (&chart_reader->timing_groups, render_ctx, arctap_renderer  , current_ms, base_bpm, scroll_speed);
}

void chart_reader_print(ChartReader *chart_reader)
{
  for (int i = 0; i < chart_reader->timing_groups.size; i++)
    timing_group_print((ChartTimingGroup *)list_get(&chart_reader->timing_groups, i));
}

void chart_reader_unload(ChartReader *chart_reader)
{
  chart_reader->initialized = false;
  for (int i = 0; i < chart_reader->timing_groups.size; i++)
    timing_group_unload((ChartTimingGroup *)list_get(&chart_reader->timing_groups, i));
}
