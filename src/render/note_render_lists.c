#include "note_render_lists.h"
#include "data/gameplay_events/beatline.h"
#include "data/gameplay_events/tap.h"
#include "data/gameplay_events/arctap.h"
#include "data/keyframe/value_channel.h"
#include "gameplay/arc_formula.h"

void render_lists_init(NoteRenderLists *render_lists)
{
  list_init(&render_lists->beatline_render_list, sizeof(BeatLine));
  list_init(&render_lists->hold_render_list, sizeof(const void *));
  list_init(&render_lists->tap_render_list, sizeof(TapFP));
  list_init(&render_lists->arc_render_list, sizeof(const void *));
  list_init(&render_lists->arccap_render_list, sizeof(const void *));
  list_init(&render_lists->arctap_render_list, sizeof(ArcTapFP));

  render_lists->enwidencamera_channel = value_channel_init();
  render_lists->enwidenlanes_channel = value_channel_init();
}

void render_lists_process(NoteRenderLists *render_lists, List *timing_groups, ChartSettings *chart_settings,
                          float current_ms, double *curr_fps, float *curr_bpms, double low_z_clip, double high_z_clip)
{
  render_lists_clear(render_lists);

  for (int i = 0; i < timing_groups->size; i++) {
    ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, i);

    double curr_fp = get_floor_position(&tg->timing_events, current_ms);
    TimingEvent *curr_event = get_event_at(&tg->timing_events, current_ms);
    float curr_bpm = curr_event != NULL ? curr_event->bpm : chart_settings->base_bpm;
    curr_fps[i]  = curr_fp;
    curr_bpms[i] = curr_bpm;

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
    itv_tree_get_overlaps(&tg->holds_tree, &render_lists->hold_render_list, curr_interval);

    // Taps
    TapFP tap_low_z_fp  = { .fp = curr_fp + low_z_clip };
    TapFP tap_high_z_fp = { .fp = curr_fp + high_z_clip };
    int tap_start_index = bisect_left(&tg->tap_fps, &tap_low_z_fp , tapfp_compare_fp_asc);
    int tap_end_index   = bisect_left(&tg->tap_fps, &tap_high_z_fp, tapfp_compare_fp_asc);
    for (int j = tap_start_index; j < tap_end_index; j++)
    {
      TapFP *tap_fp = (TapFP *)list_get(&tg->tap_fps, j);
      list_push(&render_lists->tap_render_list, tap_fp);
    }

    // Arcs
    double curr_arc_itv_arr[]  = { curr_fp + low_z_clip, curr_fp + high_z_clip };
    Interval curr_arc_interval = { .low = &curr_arc_itv_arr[0], .high = &curr_arc_itv_arr[1] };
    itv_tree_get_overlaps(&tg->arc_segments_tree, &render_lists->arc_render_list, curr_arc_interval);

    // Following arccaps
    Interval arccap_interval = { .low = &curr_fp, .high = &curr_fp };
    itv_tree_get_overlaps(&tg->arc_segments_tree, &render_lists->arccap_render_list, arccap_interval);

    // Arctaps
    ArcTapFP arctap_low_fp  = { .fp = curr_fp + low_z_clip };
    ArcTapFP arctap_high_fp = { .fp = curr_fp + high_z_clip };
    int arctap_start_index = bisect_left(&tg->arctap_fps, &arctap_low_fp , arctapfp_compare_fp_asc);
    int arctap_end_index   = bisect_left(&tg->arctap_fps, &arctap_high_fp, arctapfp_compare_fp_asc);
    for (int j = arctap_start_index; j < arctap_end_index; j++)
    {
      ArcTapFP *arctap_fp = (ArcTapFP *)list_get(&tg->arctap_fps, j);
      list_push(&render_lists->arctap_render_list, arctap_fp);
    }
  }

  list_sort_by(&render_lists->beatline_render_list, beatline_compare_fp_asc);
  list_sort_by(&render_lists->hold_render_list, arc_segment_const_void_compare_start_fp_asc);
  list_sort_by(&render_lists->tap_render_list, tapfp_compare_fp_asc);
  list_sort_by(&render_lists->arc_render_list, arc_segment_const_void_compare_start_fp_asc);
  list_sort_by(&render_lists->arctap_render_list, arctapfp_compare_fp_asc);

  list_sort_by(&render_lists->enwidencamera_channel.keyframes, value_kf_compare_start_timing_asc);
  list_sort_by(&render_lists->enwidenlanes_channel.keyframes, value_kf_compare_start_timing_asc);
}

void render_lists_clear(NoteRenderLists *render_lists)
{
  list_clear(&render_lists->beatline_render_list);
  list_clear(&render_lists->hold_render_list);
  list_clear(&render_lists->tap_render_list);
  list_clear(&render_lists->arc_render_list);
  list_clear(&render_lists->arccap_render_list);
  list_clear(&render_lists->arctap_render_list);

  list_clear(&render_lists->enwidencamera_channel.keyframes);
}

void render_lists_unload(NoteRenderLists *render_lists)
{
  list_free(&render_lists->beatline_render_list);
  list_free(&render_lists->hold_render_list);
  list_free(&render_lists->tap_render_list);
  list_free(&render_lists->arc_render_list);
  list_free(&render_lists->arccap_render_list);
  list_free(&render_lists->arctap_render_list);

  value_channel_unload(&render_lists->enwidencamera_channel);
}
