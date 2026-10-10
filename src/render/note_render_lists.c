#include <math.h>
#include "note_render_lists.h"
#include "data/gameplay_events/arc.h"
#include "data/gameplay_events/beatline.h"
#include "data/gameplay_events/hold.h"
#include "data/gameplay_events/tap.h"
#include "data/gameplay_events/arctap.h"
#include "data/keyframe/value_channel.h"
#include "gameplay/arc_formula.h"

void render_lists_init(NoteRenderLists *render_lists)
{
  list_init(&render_lists->beatline_render_list, sizeof(BeatLine));
  list_init(&render_lists->hold_render_list, sizeof(HoldRender));
  list_init(&render_lists->tap_render_list, sizeof(TapRender));
  list_init(&render_lists->arc_segment_render_list, sizeof(ArcSegmentRender));
  list_init(&render_lists->arccap_render_list, sizeof(ArcSegmentRender));
  list_init(&render_lists->arctap_render_list, sizeof(ArcTapRender));

  render_lists->enwidencamera_channel = value_channel_init();
  render_lists->enwidenlanes_channel = value_channel_init();
}

void render_lists_process(NoteRenderLists *render_lists, List *timing_groups, ChartSettings *chart_settings,
                          float current_ms, double *curr_fps, float *curr_bpms, bool *hidegroup_actives, float *groupalpha_fade,
                          double low_z_clip, double high_z_clip)
{
  render_lists_clear(render_lists);

  for (int i = 0; i < timing_groups->size; i++) {
    ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, i);

    double curr_fp = get_floor_position(&tg->timing_events, current_ms);
    TimingEvent *curr_event = get_event_at(&tg->timing_events, current_ms);
    float curr_bpm = curr_event != NULL ? curr_event->bpm : chart_settings->base_bpm;
    curr_fps[i]  = curr_fp;
    curr_bpms[i] = curr_bpm;

    tg->hidegroup_channel.current_value = value_channel_interpolate(&tg->hidegroup_channel, current_ms);
    hidegroup_actives[i] = fabsf(tg->hidegroup_channel.current_value) > 1e-6;
    tg->groupalpha_channel.current_value = value_channel_interpolate(&tg->groupalpha_channel, current_ms);
    groupalpha_fade[i] = tg->groupalpha_channel.current_value / 255.0f;

    for (int j = 0; j < tg->enwidencamera_channel.keyframes.size; j++)
    {
      ValueKeyframe *kf = (ValueKeyframe *)list_get(&tg->enwidencamera_channel.keyframes, j);
      list_push(&render_lists->enwidencamera_channel.keyframes, kf);
    }

    for (int j = 0; j < tg->enwidenlanes_channel.keyframes.size; j++)
    {
      ValueKeyframe *kf = (ValueKeyframe *)list_get(&tg->enwidenlanes_channel.keyframes, j);
      list_push(&render_lists->enwidenlanes_channel.keyframes, kf);
    }

    // Beatlines
    BeatLine beatline_low_z_fp  = { .fp = curr_fp + low_z_clip };
    BeatLine beatline_high_z_fp = { .fp = curr_fp + high_z_clip };
    int beatline_start_index = bisect_left(&tg->beatlines, &beatline_low_z_fp , beatline_compare_fp_asc);
    int beatline_end_index   = bisect_left(&tg->beatlines, &beatline_high_z_fp, beatline_compare_fp_asc);
    for (int j = beatline_start_index; j < beatline_end_index; j++)
    {
      BeatLine *beatline = (BeatLine *)list_get(&tg->beatlines, j);
      list_push(&render_lists->beatline_render_list, beatline);
    }

    // Holds
    double curr_itv_arr[] = { curr_fp + low_z_clip, curr_fp + high_z_clip };
    Interval curr_interval = { .low = &curr_itv_arr[0], .high = &curr_itv_arr[1] };
    List hold_overlaps; list_init(&hold_overlaps, sizeof(const void *));
    itv_tree_get_overlaps(&tg->holds_tree, &hold_overlaps, curr_interval);
    for (int j = 0; j < hold_overlaps.size; j++)
    {
      Hold *hold = *(Hold **)list_get(&hold_overlaps, j);
      HoldRender hold_render = { .hold_id = hold->id, .timing_group = tg->value,
                                 .start_fp = hold->start_fp, .end_fp = hold->end_fp };
      list_push(&render_lists->hold_render_list, &hold_render);
    }
    list_free(&hold_overlaps);

    // Taps
    Tap tap_low_z_fp  = { .fp = curr_fp + low_z_clip };
    Tap tap_high_z_fp = { .fp = curr_fp + high_z_clip };
    int tap_start_index = bisect_left(&tg->taps, &tap_low_z_fp , tap_compare_fp_asc);
    int tap_end_index   = bisect_left(&tg->taps, &tap_high_z_fp, tap_compare_fp_asc);
    for (int j = tap_start_index; j < tap_end_index; j++)
    {
      Tap *tap = (Tap *)list_get(&tg->taps, j);
      TapRender tap_render = { .tap_id = tap->id, .timing_group = tg->value, .fp = tap->fp };
      list_push(&render_lists->tap_render_list, &tap_render);
    }

    // Arcs
    double curr_arc_itv_arr[]  = { curr_fp + low_z_clip, curr_fp + high_z_clip };
    Interval curr_arc_interval = { .low = &curr_arc_itv_arr[0], .high = &curr_arc_itv_arr[1] };
    List arc_segments_overlaps; list_init(&arc_segments_overlaps, sizeof(const void *));
    itv_tree_get_overlaps(&tg->arc_segments_tree, &arc_segments_overlaps, curr_arc_interval);
    for(int j = 0; j < arc_segments_overlaps.size; j++)
    {
      ArcSegmentRender *segment_render = *(ArcSegmentRender **)list_get(&arc_segments_overlaps, j);
      list_push(&render_lists->arc_segment_render_list, segment_render);
    }
    list_free(&arc_segments_overlaps);

    // Following arccaps
    Interval arccap_interval = { .low = &curr_fp, .high = &curr_fp };
    List arccaps_overlaps; list_init(&arccaps_overlaps, sizeof(const void *));
    itv_tree_get_overlaps(&tg->arc_segments_tree, &arccaps_overlaps, arccap_interval);
    for(int j = 0; j < arccaps_overlaps.size; j++)
    {
      ArcSegmentRender *segment_render = *(ArcSegmentRender **)list_get(&arccaps_overlaps, j);
      list_push(&render_lists->arccap_render_list, segment_render);
    }
    list_free(&arccaps_overlaps);

    // Arctaps
    ArcTap arctap_low_fp  = { .fp = curr_fp + low_z_clip };
    ArcTap arctap_high_fp = { .fp = curr_fp + high_z_clip };
    int arctap_start_index = bisect_left(&tg->arctaps, &arctap_low_fp , arctap_compare_fp_asc);
    int arctap_end_index   = bisect_left(&tg->arctaps, &arctap_high_fp, arctap_compare_fp_asc);
    for (int j = arctap_start_index; j < arctap_end_index; j++)
    {
      ArcTap *arctap = (ArcTap *)list_get(&tg->arctaps, j);
      ArcTapRender arctap_render = { .arctap_id = arctap->id, .timing_group = tg->value, .fp = arctap->fp };
      list_push(&render_lists->arctap_render_list, &arctap_render);
    }
  }

  list_sort_by(&render_lists->beatline_render_list, beatline_compare_fp_asc);
  list_sort_by(&render_lists->hold_render_list, hold_render_compare_start_fp_asc);
  list_sort_by(&render_lists->tap_render_list, tap_render_compare_fp_asc);
  list_sort_by(&render_lists->arc_segment_render_list, arc_segment_render_compare_start_fp_asc);
  list_sort_by(&render_lists->arctap_render_list, arctap_render_compare_fp_asc);

  list_sort_by(&render_lists->enwidencamera_channel.keyframes, value_kf_compare_start_timing_asc);
  list_sort_by(&render_lists->enwidenlanes_channel.keyframes, value_kf_compare_start_timing_asc);
}

void render_lists_clear(NoteRenderLists *render_lists)
{
  list_clear(&render_lists->beatline_render_list);
  list_clear(&render_lists->hold_render_list);
  list_clear(&render_lists->tap_render_list);
  list_clear(&render_lists->arc_segment_render_list);
  list_clear(&render_lists->arccap_render_list);
  list_clear(&render_lists->arctap_render_list);

  list_clear(&render_lists->enwidencamera_channel.keyframes);
  list_clear(&render_lists->enwidenlanes_channel.keyframes);
}

void render_lists_unload(NoteRenderLists *render_lists)
{
  list_free(&render_lists->beatline_render_list);
  list_free(&render_lists->hold_render_list);
  list_free(&render_lists->tap_render_list);
  list_free(&render_lists->arc_segment_render_list);
  list_free(&render_lists->arccap_render_list);
  list_free(&render_lists->arctap_render_list);

  value_channel_unload(&render_lists->enwidencamera_channel);
  value_channel_unload(&render_lists->enwidenlanes_channel);
}
