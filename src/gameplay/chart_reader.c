#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "color_services.h"
#include "constants.h"
#include "data/custom_types/dynamic_list.h"
#include "data/gameplay_events/scenecontrol/scenecontrols.h"
#include "data/keyframe/value_channel.h"
#include "raylib.h"
#include "rlgl.h"
#include "chart_reader.h"
#include "data/chart_timing_groups/chart_timing_group.h"
#include "data/custom_types/custom_types.h"
#include "data/gameplay_events/gameplay_events.h"
#include "gameplay/arc_formula.h"
#include "render/note_render_lists.h"

static void chart_reader_rebuild_arctaps(ChartTimingGroup *tg)
{
  list_clear(&tg->arctaps);

  // Pass 1: rebuild tg->arctaps from arc->arctaps
  for (int j = 0; j < tg->arcs.size; j++)
  {
    struct Arc *arc = (struct Arc *)list_get(&tg->arcs, j);
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
    struct Arc *arc = (struct Arc *)list_get(&tg->arcs, j);
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
    case ARC:          arc_parse_aff(&tg->arcs, line, current_tg);                                       break;
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

static void parse_post_process(ChartSettings *chart_settings, AudioClock *audio_clock, ChartReader *chart_reader, Texture2D *arc_texture, Shader *arc_shader)
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

    list_sort_by(&tg->arcs, arc_compare_start_timing_asc);
    chart_reader_rebuild_arctaps(tg);

    // Beatlines
    if (tg->value == 0)
    {
      tg->beatlines = beatline_generate(tg, audio_clock->total_audio_length, chart_settings->audio_offset,
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
        if (abs(arctap->timing - tap->timing) < 2)
        {
          float x = arc_world_x_at(arctap->timing, arctap->arc);
          float y = arc_world_y_at(arctap->timing, arctap->arc);
          list_push(&tap->connector_x, &x);
          list_push(&tap->connector_y, &y);
        }
      }
    }
    list_sort_by(&tg->tap_fps, tapfp_compare_fp_asc);

    // Arcs
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
      generate_segment_meshes(chart_settings, tg, arc, arc_texture, arc_shader);
    };
    list_sort_by(&tg->arc_segments, arc_segment_compare_start_fp_asc);
    arc_segment_build_tree(&tg->arc_segments_tree, &tg->arc_segments, 0, tg->arc_segments.size - 1);

    // Arctaps
    for (int j = 0; j < tg->arctaps.size; j++)
    {
      ArcTap *arctap = (ArcTap *)list_get(&tg->arctaps, j);
      ArcTapFP arctap_fp = { .arctap = arctap, .fp = arctap->fp };
      list_push(&tg->arctap_fps, &arctap_fp);
    }
    list_sort_by(&tg->arctap_fps, arctapfp_compare_fp_asc);
  }
}

ChartReader chart_reader_parse(ChartSettings *chart_settings, AudioClock *audio_clock, Texture2D *arc_texture, Shader *arc_shader)
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

  parse_post_process(chart_settings, audio_clock, &chart_reader, arc_texture, arc_shader);
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
  render_lists_unload(&chart_reader->render_lists);
  for (int i = 0; i < chart_reader->timing_groups.size; i++)
    timing_group_unload((ChartTimingGroup *)list_get(&chart_reader->timing_groups, i));
  list_free(&chart_reader->timing_groups);
}
