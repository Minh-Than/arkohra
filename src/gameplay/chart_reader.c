#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "color_services.h"
#include "constants.h"
#include "raylib.h"
#include "rlgl.h"
#include "chart_reader.h"
#include "data/chart_timing_groups/chart_timing_group.h"
#include "data/custom_types/custom_types.h"
#include "data/gameplay_events/gameplay_events.h"
#include "gameplay/arc_formula.h"
#include "render/note_render_lists.h"
#include "render/render_service.h"

static void chart_reader_rebuild_arctaps(ChartTimingGroup *tg)
{
  list_clear(&tg->arctaps);

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

static void parse_aff_lines(char *line, ChartReader *chart_reader, int *tg_count, int *current_tg)
{
  ChartTimingGroup *tg = (ChartTimingGroup *)list_get(&chart_reader->timing_groups, *current_tg);
  RawEventType type = determine_type(line);
  switch (type)
  {
    case TIMING_GROUP:
      {
        *tg_count += 1;
        *current_tg = *tg_count - 1;
        ChartTimingGroup init_tg = timing_group_init();
        init_tg.value = *current_tg;
        parse_tg_props(line, &init_tg);
        list_push(&chart_reader->timing_groups, &init_tg);
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

    // Beatlines
    if (tg->value == 0)
    {
      tg->beatlines = beatline_generate(tg, render_ctx->audio_clock.total_audio_length, render_ctx->chart_settings.audio_offset,
                                        color_from_rgba(BEATLINE_CL), BEATLINE_THICKNESS);
      list_sort_by(&tg->beatlines, beatline_compare_fp_asc);
    }

    // Pre-calculate the notes's floor position
    // Arctaps' floor position is already calculated in `chart_reader_rebuild_arctaps`
    for (int j = 0; j < tg->holds.size; j++)
    {
      Hold *hold = (Hold *)list_get(&tg->holds, j);
      hold->start_fp = get_floor_position(&tg->timing_events, hold->start_timing);
      hold->end_fp   = get_floor_position(&tg->timing_events, hold->end_timing);
    }
    list_sort_by(&tg->holds, hold_compare_start_fp_asc);
    hold_build_tree(&tg->holds_tree, &tg->holds, 0, tg->holds.size - 1);

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

      Arc low_target  = { .start_timing = arc->end_timing - 1 };
      Arc high_target = { .start_timing = arc->end_timing + 1 };
      int start_idx = bisect_left(&tg->arcs, &low_target, arc_compare_start_timing_asc);
      int end_idx = bisect_right(&tg->arcs, &high_target, arc_compare_start_timing_asc);
      for (int k = start_idx; k < end_idx; k++)
      {
        Arc *connected_arc = (Arc *)list_get(&tg->arcs, k);
        if (fabsf(connected_arc->x1 - arc->x2) > 1e-6 ||
            fabsf(connected_arc->y1 - arc->y2) > 1e-6) continue;
        if (!(connected_arc->is_void ^ arc->is_void)) connected_arc->is_head = false;
      }
      generate_segments(&tg->arc_segments, arc, arc_texture, &tg->timing_events, render_ctx);
      shadow_segment_generate_mesh(&tg->arc_segments, arc, &tg->timing_events, render_ctx);
    };
    list_sort_by(&tg->arc_segments, arc_segment_compare_start_fp_asc);
    arc_segment_build_tree(&tg->arc_segments_tree, &tg->arc_segments, 0, tg->arc_segments.size - 1);

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

  chart_reader.render_lists = (NoteRenderLists){ 0 };
  render_lists_initialize(&chart_reader.render_lists);

  chart_reader.low_z_clip  = z_to_floor_position(   9.0f, render_ctx->chart_settings.base_bpm, render_ctx->chart_settings.scroll_speed);
  chart_reader.high_z_clip = z_to_floor_position(-100.0f, render_ctx->chart_settings.base_bpm, render_ctx->chart_settings.scroll_speed);

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
  if (lines != NULL) UnloadFileText(aff_data);

  parse_post_process(render_ctx, &chart_reader, arc_texture);
  chart_reader.initialized = true;

  for (int i = 0; i < chart_reader.timing_groups.size; i++)
  {
    ChartTimingGroup *tg = (ChartTimingGroup *)list_get(&chart_reader.timing_groups, i);
    timing_group_print(tg);
  }
  return chart_reader;
}

static void process_note_render_lists(ChartReader *chart_reader, RenderContext *render_ctx, float current_ms, double *curr_fps, float *curr_bpms)
{
  render_lists_clear(&chart_reader->render_lists);

  List *timing_groups = &chart_reader->timing_groups;
  for (int i = 0; i < timing_groups->size; i++) {
    ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, i);

    double curr_fp = get_floor_position(&tg->timing_events, current_ms);
    TimingEvent *curr_event = get_event_at(&tg->timing_events, current_ms);
    float curr_bpm = curr_event != NULL ? curr_event->bpm : render_ctx->chart_settings.base_bpm;
    curr_fps[i]  = curr_fp;
    curr_bpms[i] = curr_bpm;

    // Beatlines
    BeatLine beatline_low_z_fp  = { .fp = curr_fp + chart_reader->low_z_clip };
    BeatLine beatline_high_z_fp = { .fp = curr_fp + chart_reader->high_z_clip };
    int beatline_start_index = bisect_left(&tg->beatlines, &beatline_low_z_fp , beatline_compare_fp_asc);
    int beatline_end_index   = bisect_left(&tg->beatlines, &beatline_high_z_fp, beatline_compare_fp_asc);
    for (int j = beatline_start_index; j < beatline_end_index; j++)
    {
      BeatLine *beatline = (BeatLine *)list_get(&tg->beatlines, j);
      list_push(&chart_reader->render_lists.beatline_render_list, beatline);
    }

    // Holds
    double curr_itv_arr[] = { curr_fp + chart_reader->low_z_clip, curr_fp + chart_reader->high_z_clip };
    Interval curr_interval = { .low = &curr_itv_arr[0], .high = &curr_itv_arr[1] };
    itv_tree_get_overlaps(&tg->holds_tree, &chart_reader->render_lists.hold_render_list, curr_interval);

    // Taps
    TapFP tap_low_z_fp  = { .fp = curr_fp + chart_reader->low_z_clip };
    TapFP tap_high_z_fp = { .fp = curr_fp + chart_reader->high_z_clip };
    int tap_start_index = bisect_left(&tg->tap_fps, &tap_low_z_fp , tapfp_compare_fp_asc);
    int tap_end_index   = bisect_left(&tg->tap_fps, &tap_high_z_fp, tapfp_compare_fp_asc);
    for (int j = tap_start_index; j < tap_end_index; j++)
    {
      TapFP *tap_fp = (TapFP *)list_get(&tg->tap_fps, j);
      list_push(&chart_reader->render_lists.tap_render_list, tap_fp);
    }

    // Arcs
    double curr_arc_itv_arr[]  = { curr_fp + chart_reader->low_z_clip, curr_fp + chart_reader->high_z_clip };
    Interval curr_arc_interval = { .low = &curr_arc_itv_arr[0], .high = &curr_arc_itv_arr[1] };
    itv_tree_get_overlaps(&tg->arc_segments_tree, &chart_reader->render_lists.arc_render_list, curr_arc_interval);

    // Following arccaps
    Interval arccap_interval = { .low = &curr_fp, .high = &curr_fp };
    itv_tree_get_overlaps(&tg->arc_segments_tree, &chart_reader->render_lists.arccap_render_list, arccap_interval);

    // Arctaps
    ArcTapFP arctap_low_fp  = { .fp = curr_fp + chart_reader->low_z_clip };
    ArcTapFP arctap_high_fp = { .fp = curr_fp + chart_reader->high_z_clip };
    int arctap_start_index = bisect_left(&tg->arctap_fps, &arctap_low_fp , arctapfp_compare_fp_asc);
    int arctap_end_index   = bisect_left(&tg->arctap_fps, &arctap_high_fp, arctapfp_compare_fp_asc);
    for (int j = arctap_start_index; j < arctap_end_index; j++)
    {
      ArcTapFP *arctap_fp = (ArcTapFP *)list_get(&tg->arctap_fps, j);
      list_push(&chart_reader->render_lists.arctap_render_list, arctap_fp);
    }
  }

  list_sort_by(&chart_reader->render_lists.beatline_render_list, beatline_compare_fp_asc);
  list_sort_by(&chart_reader->render_lists.hold_render_list, arc_segment_const_void_compare_start_fp_asc);
  list_sort_by(&chart_reader->render_lists.tap_render_list, tapfp_compare_fp_asc);
  list_sort_by(&chart_reader->render_lists.arc_render_list, arc_segment_const_void_compare_start_fp_asc);
  list_sort_by(&chart_reader->render_lists.arctap_render_list, arctapfp_compare_fp_asc);
}

void chart_reader_render_notes(RenderContext *render_ctx, ChartReader* chart_reader,
                               HoldTapRenderer *hold_tap_renderer, ArcRenderer *arc_renderer, ArctapRenderer *arctap_renderer,
                               float current_ms)
{
  if (!chart_reader->initialized) return;

  float base_bpm     = render_ctx->chart_settings.base_bpm;
  float scroll_speed = render_ctx->chart_settings.scroll_speed;
  double curr_fps[chart_reader->timing_groups.size];
  float curr_bpms[chart_reader->timing_groups.size];

  process_note_render_lists(chart_reader, render_ctx, current_ms, curr_fps, curr_bpms);
  List *tgs           = &chart_reader->timing_groups;
  NoteRenderLists *ls = &chart_reader->render_lists;

  render_holds_taps      (tgs, ls, render_ctx, hold_tap_renderer, current_ms, base_bpm, scroll_speed, curr_fps);
  render_arcs_and_shadows(tgs, ls, render_ctx, arc_renderer     , current_ms, base_bpm, scroll_speed, curr_fps, curr_bpms);
  render_arctaps         (tgs, ls, render_ctx, arctap_renderer  , current_ms, base_bpm, scroll_speed, curr_fps, curr_bpms);
}

void chart_reader_print(ChartReader *chart_reader)
{
  for (int i = 0; i < chart_reader->timing_groups.size; i++)
    timing_group_print((ChartTimingGroup *)list_get(&chart_reader->timing_groups, i));
}

void chart_reader_unload(ChartReader *chart_reader)
{
  chart_reader->initialized = false;
  render_lists_unload(&chart_reader->render_lists);
  for (int i = 0; i < chart_reader->timing_groups.size; i++)
    timing_group_unload((ChartTimingGroup *)list_get(&chart_reader->timing_groups, i));
  list_free(&chart_reader->timing_groups);
}
