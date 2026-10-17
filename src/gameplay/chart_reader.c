#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "color_services.h"
#include "constants.h"
#include "raylib.h"
#include "rlgl.h"
#include "chart_reader.h"
#include "data/custom_types/dynamic_list.h"
#include "data/custom_types/custom_types.h"
#include "data/chart_timing_groups/chart_timing_group.h"
#include "data/gameplay_events/scenecontrol/scenecontrols.h"
#include "data/keyframe/value_channel.h"
#include "data/gameplay_events/gameplay_events.h"
#include "gameplay/arc_formula.h"
#include "render/note_render_lists.h"

static bool parse_aff_header(char *line, ChartSettings *chart_settings)
{
  bool end_of_header = strcmp(line, "-") == 0;

  if (strstr(line, "AudioOffset") != NULL)
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
    case TIMING_GROUP: timing_group_parse_aff(&chart_reader->timing_groups, line, tg_count, current_tg); break;
    case TIMING_EVENT: timing_event_parse_aff(&tg->timing_events, line, current_tg);                     break;
    case TAP:          tap_parse_aff(&tg->taps, line, current_tg);                                       break;
    case HOLD:         hold_parse_aff(&tg->holds, line, current_tg);                                     break;
    case ARC:          arc_parse_aff(&tg->arcs, &tg->arctaps, line, current_tg, &chart_reader->arc_counter);break;
    case SCENECONTROL: scenecontrol_parse_aff(tg, line);                                                 break;
    default: break;
  }
}

static void post_process_channel(ValueChannel *channel)
{
  int prev_value = 0;
  list_sort_by(&channel->keyframes, value_kf_compare_start_timing_asc);
  for (int j = 0; j < channel->keyframes.size; j++)
  {
    ValueKeyframe *kf = (ValueKeyframe *)list_get(&channel->keyframes, j);
    if (j == 0) { prev_value = kf->next_value; continue; }
    kf->prev_value = prev_value;
    prev_value = kf->next_value;
  }
}

static void post_process_arg_segments(ChartSettings *chart_settings, ChartTimingGroup *tg, struct Arc *arc)
{
  List *timing_events  = &tg->timing_events;

  int arc_duration      = arc->end_timing - arc->start_timing;
  float segment_length  = calculate_arc_segment_length(arc_duration, arc->arc_res * tg->props.arc_res);
  int segment_count     = (int)ceilf(arc_duration / segment_length);
  segment_count         = (int)fmax(segment_count, 1);

  float segment_timing = arc->start_timing;
  float curr_timing    = 0.0f;
  for (int i = 0; i < segment_count; i++)
  {
    float increment     = fminf(arc->end_timing - arc->start_timing - curr_timing, segment_length);
    bool zero_duration  = (arc->end_timing == arc->start_timing);
    struct Arc temp_arc = {
      .x1 = arc->x1, .y1 = arc->y1,
      .x2 = arc->x2, .y2 = arc->y2,
      .start_timing  = 0,
      .end_timing    = arc->end_timing - arc->start_timing,
      .type          = arc->type
    };

    float segment_start_timing = curr_timing + arc->start_timing;
    double z_start = floor_position_to_z(get_floor_position(timing_events, segment_start_timing),
                                         chart_settings->base_bpm,
                                         chart_settings->scroll_speed);
    double z_end   = floor_position_to_z(get_floor_position(timing_events, segment_start_timing + increment),
                                         chart_settings->base_bpm,
                                         chart_settings->scroll_speed);

    ArcSegmentRender segment_render = {
      .arc_id           = arc->id,
      .timing_group     = arc->timing_group,
      .start_fp         = get_floor_position(timing_events, curr_timing + arc->start_timing),
      .end_fp           = get_floor_position(timing_events, curr_timing + increment + arc->start_timing),
      .start_timing     = segment_timing,
      .end_timing       = segment_timing + increment,
      .arc_start_timing = arc->start_timing,
      .arc_end_timing   = arc->end_timing,
      .world_x1         = zero_duration ? arc_x_to_world(arc->x1) : arc_world_x_at(curr_timing, &temp_arc),
      .world_y1         = zero_duration ? arc_y_to_world(arc->y1) : arc_world_y_at(curr_timing, &temp_arc),
      .world_x2         = zero_duration ? arc_x_to_world(arc->x2) : arc_world_x_at(curr_timing + increment, &temp_arc),
      .world_y2         = zero_duration ? arc_y_to_world(arc->y2) : arc_world_y_at(curr_timing + increment, &temp_arc),
      .z_delta          = z_end - z_start,
    };
    list_push(&tg->arc_segments, &segment_render);

    segment_timing += increment;
    curr_timing += increment;
  }
}

static void parse_post_process(ChartSettings *chart_settings, AudioClock *audio_clock, ChartReader *chart_reader)
{
  for(int i = 0; i < chart_reader->timing_groups.size; i++)
  {
    ChartTimingGroup *tg = (ChartTimingGroup *)list_get(&chart_reader->timing_groups, i);
    list_sort_by(&tg->timing_events, timing_event_compare_timing_asc);
    recalculate_floor_position(tg);

    post_process_channel(&tg->hidegroup_channel);
    post_process_channel(&tg->groupalpha_channel);
    post_process_channel(&tg->enwidencamera_channel);
    post_process_channel(&tg->enwidenlanes_channel);

    // Beatlines
    if (tg->value == 0)
    {
      tg->beatlines = beatline_generate(tg, audio_clock->total_audio_length, chart_settings->audio_offset,
                                        color_from_rgba(BEATLINE_CL), BEATLINE_THICKNESS);
      list_sort_by(&tg->beatlines, beatline_compare_fp_asc);
    }

    // Arcs
    list_sort_by(&tg->arcs, arc_compare_start_timing_asc);
    for (int j = 0; j < tg->arcs.size; j++)
    {
      struct Arc *arc = (struct Arc *)list_get(&tg->arcs, j);
      arc->start_fp = get_floor_position(&tg->timing_events, arc->start_timing);
      arc->end_fp   = get_floor_position(&tg->timing_events, arc->end_timing);

      struct Arc low_target  = { .start_timing = arc->end_timing - 1 };
      struct Arc high_target = { .start_timing = arc->end_timing + 1 };
      int start_idx = bisect_left(&tg->arcs, &low_target, arc_compare_start_timing_asc);
      int end_idx = bisect_right(&tg->arcs, &high_target, arc_compare_start_timing_asc);
      for (int k = start_idx; k < end_idx; k++)
      {
        struct Arc *connected_arc = (struct Arc *)list_get(&tg->arcs, k);
        if (connected_arc == arc) continue;
        if (fabsf(connected_arc->x1 - arc->x2) > 1e-6 ||
            fabsf(connected_arc->y1 - arc->y2) > 1e-6) continue;

        if (!(connected_arc->is_void ^ arc->is_void))
        {
          connected_arc->is_head = false;
          arc->next_arc = connected_arc;
          connected_arc->prev_arc = arc;
        }
      }
      post_process_arg_segments(chart_settings, tg, arc);
    };
    list_sort_by(&tg->arc_segments, arc_segment_render_compare_start_fp_asc);
    arc_segment_render_build_tree(&tg->arc_segments_tree, &tg->arc_segments, 0, tg->arc_segments.size - 1);

    // Arctaps
    for (int j = 0; j < tg->arctaps.size; j++)
    {
      ArcTap *arctap = (ArcTap *)list_get(&tg->arctaps, j);
      arctap->id = chart_reader->arctap_counter++;
      arctap->fp = get_floor_position(&tg->timing_events, arctap->timing);
    }
    list_sort_by(&tg->arctaps, arctap_compare_fp_asc);

    // Holds
    for (int j = 0; j < tg->holds.size; j++)
    {
      Hold *hold = (Hold *)list_get(&tg->holds, j);
      hold->id  = chart_reader->hold_counter++;
      hold->start_fp = get_floor_position(&tg->timing_events, hold->start_timing);
      hold->end_fp   = get_floor_position(&tg->timing_events, hold->end_timing);
    }
    list_sort_by(&tg->holds, hold_compare_start_fp_asc);
    hold_build_tree(&tg->holds_tree, &tg->holds, 0, tg->holds.size - 1);

    // Taps
    for (int j = 0; j < tg->taps.size; j++)
    {
      Tap *tap = (Tap *)list_get(&tg->taps, j);
      tap->id  = chart_reader->tap_counter++;
      tap->fp  = get_floor_position(&tg->timing_events, tap->timing);

      for (int k = 0; k < tg->arctaps.size; k++)
      {
        ArcTap *arctap = (ArcTap *)list_get(&tg->arctaps, k);
        if (abs(arctap->timing - tap->timing) < 2)
        {
          struct Arc compare_arc = { .id = arctap->arc_id };
          struct Arc *arc = (struct Arc *)list_find(&tg->arcs, &compare_arc, arc_compare_id_eq);
          if (arc != NULL)
          {
            float x = arc_world_x_at(arctap->timing, arc);
            float y = arc_world_y_at(arctap->timing, arc);
            list_push(&tap->connector_x, &x);
            list_push(&tap->connector_y, &y);
          }
        }
      }
    }
    list_sort_by(&tg->taps, tap_compare_fp_asc);
  }
}

ChartReader chart_reader_parse(ChartSettings *chart_settings, AudioClock *audio_clock)
{
  ChartReader chart_reader = { 0 };

  char *aff_data = LoadFileText(chart_settings->chart_path);
  if (aff_data == NULL) return chart_reader;

  List tgs; list_init(&tgs, sizeof(ChartTimingGroup));
  ChartTimingGroup init_tg = timing_group_init();
  list_push(&tgs, &init_tg);
  chart_reader.timing_groups = tgs;

  chart_reader.render_lists = (NoteRenderLists){ 0 };
  render_lists_init(&chart_reader.render_lists);

  chart_reader.low_z_clip  = z_to_floor_position(   9.0, chart_settings->base_bpm, chart_settings->scroll_speed);
  chart_reader.high_z_clip = z_to_floor_position(-100.0, chart_settings->base_bpm, chart_settings->scroll_speed);

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
      if (parse_aff_header(line, chart_settings)) is_header = false;
      continue;
    }

    if(strcmp(line, "};")  == 0) { current_tg = 0; continue; }

    parse_aff_lines(line, &chart_reader, &tg_count, &current_tg);
  }

  if (lines != NULL) UnloadFileText(aff_data);

  parse_post_process(chart_settings, audio_clock, &chart_reader);
  chart_reader_print(&chart_reader);

  chart_reader.initialized = true;
  return chart_reader;
}

void chart_reader_print(ChartReader *chart_reader)
{
  for (int i = 0; i < chart_reader->timing_groups.size; i++)
    timing_group_info_print((ChartTimingGroup *)list_get(&chart_reader->timing_groups, i));
}

void chart_reader_unload(ChartReader *chart_reader)
{
  chart_reader->initialized = false;
  chart_reader->tap_counter = 0;
  chart_reader->hold_counter = 0;
  chart_reader->arc_counter = 0;
  chart_reader->arctap_counter = 0;
  render_lists_unload(&chart_reader->render_lists);
  for (int i = 0; i < chart_reader->timing_groups.size; i++)
    timing_group_unload((ChartTimingGroup *)list_get(&chart_reader->timing_groups, i));
  list_free(&chart_reader->timing_groups);
}
