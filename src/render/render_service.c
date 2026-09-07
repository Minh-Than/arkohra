#include <math.h>
#include <stdlib.h>
#include "render_service.h"
#include "color_services.h"
#include "constants.h"
#include "data/app_configs/app_config.h"
#include "data/chart_timing_groups/chart_timing_group.h"
#include "data/custom_types/custom_types.h"
#include "data/gameplay_events/arc.h"
#include "data/gameplay_events/arctap.h"
#include "data/gameplay_events/gameplay_events.h"
#include "data/gameplay_events/tap.h"
#include "gameplay/arc_formula.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

void render_holds_taps(List *timing_groups, RenderContext *render_ctx, HoldTapRenderer *hold_tap_renderer,
                       float current_ms, float base_bpm, float scroll_speed)
{
  List hold_render_list; list_init(&hold_render_list, sizeof(const void *));
  List tap_render_list; list_init(&tap_render_list, sizeof(TapFP));
  double low_z_clip  = z_to_floor_position(9.0f, base_bpm, scroll_speed);
  double high_z_clip = z_to_floor_position(-100.0f, base_bpm, scroll_speed);
  BeginMode3D(render_ctx->camera);
    rlDisableDepthTest();
    rlPushMatrix();
      rlScalef(1.7896f, 1.0f, 1.0f);
      rlDisableBackfaceCulling();
      BeginBlendMode(BLEND_ALPHA);
        for (int i = 0; i < timing_groups->size; i++) {
          ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, i);
          double curr_fp  = get_floor_position(&tg->timing_events, current_ms);

          double curr_itv_arr[] = { curr_fp + low_z_clip, curr_fp + high_z_clip };
          Interval curr_interval = { .low = &curr_itv_arr[0], .high = &curr_itv_arr[1] };
          itv_tree_get_overlaps(&tg->holds_tree, &hold_render_list, curr_interval);

          TapFP tap_low_z_fp  = { .fp = curr_fp + low_z_clip };
          TapFP tap_high_z_fp = { .fp = curr_fp + high_z_clip };
          int tap_start_index = bisect_left(&tg->tap_fps, &tap_low_z_fp , tapfp_compare_fp_asc);
          int tap_end_index   = bisect_left(&tg->tap_fps, &tap_high_z_fp, tapfp_compare_fp_asc);
          for (int j = tap_start_index; j < tap_end_index; j++)
          {
            TapFP *tap_fp = (TapFP *)list_get(&tg->tap_fps, j);
            list_push(&tap_render_list, tap_fp);
          }
        }

        // Precalculate current floor positions and BPMs for reusing
        int tg_size = timing_groups->size;
        double curr_fps[tg_size];
        for (int i = 0; i < tg_size; i++)
        {
          ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, i);
          curr_fps[i]  = get_floor_position(&tg->timing_events, current_ms);
        }

        // Iterate & draw
        // Holds
        list_sort_by(&hold_render_list, arc_segment_const_void_compare_start_fp_asc);
        list_sort_by(&tap_render_list, tapfp_compare_fp_asc);
        for(int i = hold_render_list.size - 1; i >= 0; i--)
        {
            Hold *hold = *(Hold **)list_get(&hold_render_list, i);
            double curr_fp = curr_fps[hold->timing_group];
            float z_pos   = floor_position_to_z(hold->start_fp - curr_fp, base_bpm, scroll_speed);
            float z_scale = floor_position_to_z(hold->end_fp - hold->start_fp, base_bpm, scroll_speed);
            if ((z_pos < -100.0f && z_scale < -100.0f) || (z_pos > 9.0f && z_scale > 9.0f)) continue;
            float alpha   = hold->start_timing < current_ms && !hold->is_active ? 0.5f : 1.0f;
            hold_render_test(&hold_tap_renderer->hold, hold, z_pos, z_scale, alpha);
        }

        // Taps
        for(int i = tap_render_list.size - 1; i >= 0; i--)
        {
            TapFP *tap_fp = (TapFP *)list_get(&tap_render_list, i);
            Tap *tap      = tap_fp->tap;
            double curr_fp = curr_fps[tap->timing_group];
            double diff_fp = tap->fp - curr_fp;
            float z_pos   = floor_position_to_z(diff_fp, base_bpm, scroll_speed);
            float z_scale = Clamp(Lerp(1.8f, 5.8f,
                                       floor_position_to_z(diff_fp, base_bpm, scroll_speed) / -100.0f),
                                  1.8f, 5.8f);
            tap_render_test(&hold_tap_renderer->tap, tap, z_pos, z_scale);

            for (int k = 0; k < tap->connector_x.size; k++)
            {
              float x = *(float *)list_get(&tap->connector_x, k);
              float y = *(float *)list_get(&tap->connector_y, k);
              DrawThickLine3D((Vector3){ lane_to_world_x(tap->lane), 0.0f, z_pos - 0.1f },
                              (Vector3){ x, y - 0.21f, z_pos - 0.1f },
                              Lerp(0.04f, 0.11f, floor_position_to_z(diff_fp, base_bpm, scroll_speed) / -100.0f),
                              render_ctx->chart_settings.skin_side == SK_CONFLICT
                                ? color_from_rgba(CONFICT_CONNECTOR_CL)
                                : color_from_rgba(LIGHT_CONNECTOR_CL)
                              );
            }
        }
      EndBlendMode();
      rlEnableBackfaceCulling();
    rlPopMatrix();
    rlEnableDepthTest();
  EndMode3D();
}

// Apparently separate texture fucks a lot of color blending in the process
void render_arcs_and_shadows(List *timing_groups, RenderContext *render_ctx, ArcRenderer *arc_renderer,
                             float current_ms, float base_bpm, float scroll_speed)
{
  List arc_render_list;    list_init(&arc_render_list, sizeof(const void *));
  List arccap_render_list; list_init(&arccap_render_list, sizeof(const void *));
  double low_z_clip    = z_to_floor_position(9.0f, base_bpm, scroll_speed);
  double high_z_clip   = z_to_floor_position(-100.0f, base_bpm, scroll_speed);
  double arccap_z_clip = z_to_floor_position(0.0f, base_bpm, scroll_speed);
  BeginMode3D(render_ctx->camera);
    rlDisableDepthTest();
    rlPushMatrix();
      rlScalef(1.7896f, 1.0f, 1.0f);
      BeginBlendMode(BLEND_ALPHA);
      rlDisableBackfaceCulling();
      for (int i = 0; i < timing_groups->size; i++)
      {
        ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, i);
        double curr_fp = get_floor_position(&tg->timing_events, current_ms);
        float curr_event_bpm = get_event_at(&tg->timing_events, current_ms)->bpm;

        double curr_itv_arr[]   = { curr_fp + low_z_clip, curr_fp + high_z_clip };
        Interval curr_interval = { .low = &curr_itv_arr[0], .high = &curr_itv_arr[1] };
        itv_tree_get_overlaps(&tg->arc_segments_tree, &arc_render_list, curr_interval);

        double arccap_itv_arr[] = { curr_fp + arccap_z_clip, curr_fp + arccap_z_clip };
        Interval arccap_interval = { .low = &arccap_itv_arr[0], .high = &arccap_itv_arr[1] };
        itv_tree_get_overlaps(&tg->arc_segments_tree, &arccap_render_list, arccap_interval);

        // Arctap shadows
        ArcTapFP arctap_low_fp  = { .fp = curr_fp + low_z_clip };
        ArcTapFP arctap_high_fp = { .fp = curr_fp + high_z_clip };
        int arctap_start_index = bisect_left(&tg->arctap_fps, &arctap_low_fp , arctapfp_compare_fp_asc);
        int arctap_end_index   = bisect_left(&tg->arctap_fps, &arctap_high_fp, arctapfp_compare_fp_asc);

        for (int j = arctap_end_index - 1; j >= arctap_start_index; j--)
        {
          ArcTapFP *arctap_fp = (ArcTapFP *)list_get(&tg->arctap_fps, j);
          ArcTap *arctap = arctap_fp->arctap;
          float arctap_z_pos = floor_position_to_z(arctap->fp - curr_fp, base_bpm, scroll_speed);
          Matrix arctap_mt   = MatrixMultiply(MatrixRotateX(-180.0f * DEG2RAD),
                                              MatrixTranslate(arc_world_x_at(arctap->timing, arctap->arc),
                                                              0.0f, arctap_z_pos));
          DrawMesh(arc_renderer->arctap_shadow.mesh, arc_renderer->arctap_shadow.material, arctap_mt);
        }
      }
      list_sort_by(&arc_render_list, arc_segment_const_void_compare_start_fp_asc);

      // Precalculate current floor positions and BPMs for reusing
      int tg_size = timing_groups->size;
      double curr_fps[tg_size]; float curr_bpms[tg_size]; // won't recommend this
      for (int i = 0; i < tg_size; i++)
      {
        ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, i);
        curr_fps[i]  = get_floor_position(&tg->timing_events, current_ms);
        curr_bpms[i] = get_event_at(&tg->timing_events, current_ms)->bpm;
      }

      // Iterate & draw
      // Arc/Trace shadows
      for(int i = arc_render_list.size - 1; i >= 0; i--)
      {
        ArcSegment *arc_segment = *(ArcSegment **)list_get(&arc_render_list, i);
        Arc *arc = arc_segment->arc;
        double curr_fp = curr_fps[arc->timing_group];
        float curr_bpm = curr_bpms[arc->timing_group];
        float z_pos    = floor_position_to_z(arc_segment->start_fp - curr_fp, base_bpm, scroll_speed);

        // Settings up shader
        Vector4 shadow_tint = ColorNormalize(color_from_rgba(NOTE_SHADOW_CL));
        int is_void_shader = arc->is_void ? 1 : 0;
        int should_clip_shader = arc->start_timing - current_ms <= 0 ? 1 : 0;
        int negative_bpm_shader = curr_bpm < 0.0f;
        SetShaderValue(render_ctx->arc_shader.shader, render_ctx->arc_shader.isVoid_loc     , &is_void_shader     , SHADER_UNIFORM_INT);
        SetShaderValue(render_ctx->arc_shader.shader, render_ctx->arc_shader.shouldClip_loc , &should_clip_shader , SHADER_UNIFORM_INT);
        SetShaderValue(render_ctx->arc_shader.shader, render_ctx->arc_shader.negativeBPM_loc, &negative_bpm_shader, SHADER_UNIFORM_INT);
        SetShaderValue(render_ctx->arc_shader.shader, render_ctx->arc_shader.tintLow_loc , &shadow_tint, SHADER_UNIFORM_VEC4);
        SetShaderValue(render_ctx->arc_shader.shader, render_ctx->arc_shader.tintHigh_loc, &shadow_tint, SHADER_UNIFORM_VEC4);
        DrawMesh(arc_segment->shadow_r.mesh, arc_segment->shadow_r.material, MatrixTranslate(0.0f, 0.0f, z_pos));
      }

      // Following Arccaps
      for (int i = 0; i < arccap_render_list.size; i++)
      {
        ArcSegment *arc_segment = *(ArcSegment **)list_get(&arccap_render_list, i);
        Arc *arc = arc_segment->arc;

        float arccap_x = arc_world_x_at(current_ms, arc);
        float arccap_y = arc_world_y_at(current_ms, arc);
        float arccap_scale = arc->is_void ? ARCCAP_TRACE_SCALE : ARCCAP_ARC_SCALE;
        Matrix tr = MatrixMultiply(MatrixScale(arccap_scale, arccap_scale, 1.0f), MatrixTranslate(arccap_x, arccap_y, 0.0f));
        DrawMesh(arc_renderer->arccap.mesh, arc_renderer->arccap.material, tr);
      }

      // Height indicators + Arcs/Traces
      for(int i = arc_render_list.size - 1; i >= 0; i--)
      {
        ArcSegment *arc_segment = *(ArcSegment **)list_get(&arc_render_list, i);
        Arc *arc = arc_segment->arc;
        double curr_fp = curr_fps[arc->timing_group];
        float curr_bpm = curr_bpms[arc->timing_group];
        float z_pos    = floor_position_to_z(arc_segment->start_fp - curr_fp, base_bpm, scroll_speed);

        // Settings up shader
        int is_void_shader = arc->is_void ? 1 : 0;
        int should_clip_shader = arc->start_timing - current_ms <= 0 ? 1 : 0;
        int negative_bpm_shader = curr_bpm < 0.0f;
        SetShaderValue(render_ctx->arc_shader.shader, render_ctx->arc_shader.isVoid_loc     , &is_void_shader     , SHADER_UNIFORM_INT);
        SetShaderValue(render_ctx->arc_shader.shader, render_ctx->arc_shader.shouldClip_loc , &should_clip_shader , SHADER_UNIFORM_INT);
        SetShaderValue(render_ctx->arc_shader.shader, render_ctx->arc_shader.negativeBPM_loc, &negative_bpm_shader, SHADER_UNIFORM_INT);

        // Height indicators
        float arc_world_x1 = arc_x_to_world(arc->x1);
        float arc_world_y1 = arc_y_to_world(arc->y1);
        if (!arc->is_void && fabs(arc_segment->start_fp - arc->start_fp) < 1e-6 && (arc->is_head || fabsf(arc->y1 - arc->y2) > 1e-6) )
        {
          rlDisableDepthMask();
          arc_renderer->height_indicator.material.maps->color = arc->color == 0 ? color_from_rgba(ARC_BLUE_HIGH_CL)
                                                                                : color_from_rgba(ARC_PINK_HIGH_CL);
          DrawMesh(arc_renderer->height_indicator.mesh, arc_renderer->height_indicator.material,
                   MatrixMultiply(MatrixRotateX(-90.0f * DEG2RAD), MatrixMultiply(MatrixScale(1.0f, arc_world_y1, 1.0f),
                                                                                  MatrixTranslate(arc_world_x1, arc_world_y1 * 0.5f, z_pos))));
          rlEnableDepthMask();
        }

        // Arcs/Traces
        Vector4 tint_low = ColorNormalize(color_from_rgba(TRACE_CL)); // Default trace tint
        Vector4 tint_high = tint_low;
        if (!arc->is_void)
        {
          tint_low  = ColorNormalize(color_from_rgba( arc->color == 0 ? ARC_BLUE_LOW_CL : ARC_PINK_LOW_CL));
          tint_high = ColorNormalize(color_from_rgba( arc->color == 0 ? ARC_BLUE_HIGH_CL : ARC_PINK_HIGH_CL));
        }
        SetShaderValue(render_ctx->arc_shader.shader, render_ctx->arc_shader.tintLow_loc , &tint_low , SHADER_UNIFORM_VEC4);
        SetShaderValue(render_ctx->arc_shader.shader, render_ctx->arc_shader.tintHigh_loc, &tint_high, SHADER_UNIFORM_VEC4);
        DrawMesh(arc_segment->mesh_r.mesh, arc_segment->mesh_r.material, MatrixTranslate(0.0f, 0.0f, z_pos));
      }
      rlEnableBackfaceCulling();
      EndBlendMode();
    rlPopMatrix();
    rlEnableDepthTest();
  EndMode3D();
  list_free(&arccap_render_list);
  list_free(&arc_render_list);
}

void render_arctaps(List *timing_groups, RenderContext *render_ctx, ArctapRenderer *arctap_renderer,
                    float current_ms, float base_bpm, float scroll_speed)
{
  List arc_head_render_list; list_init(&arc_head_render_list, sizeof(const void *));
  List arctap_render_list; list_init(&arctap_render_list, sizeof(ArcTapFP));
  double low_z_clip  = z_to_floor_position(9.0f, base_bpm, scroll_speed);
  double high_z_clip = z_to_floor_position(-100.0f, base_bpm, scroll_speed);
  BeginMode3D(render_ctx->camera);
    rlPushMatrix();
      rlScalef(1.7896f, 1.0f, 1.0f);
      rlDisableBackfaceCulling();
      for (int i = 0; i < timing_groups->size; i++)
      {
        ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, i);
        double curr_fp = get_floor_position(&tg->timing_events, current_ms);

        double curr_itv_arr[] = { curr_fp + low_z_clip, curr_fp + high_z_clip };
        Interval curr_interval = { .low = &curr_itv_arr[0], .high = &curr_itv_arr[1] };
        itv_tree_get_overlaps(&tg->arc_segments_tree, &arc_head_render_list, curr_interval);

        ArcTapFP arctap_low_z_fp  = { .fp = curr_fp + low_z_clip };
        ArcTapFP arctap_high_z_fp = { .fp = curr_fp + high_z_clip };
        int arctap_start_index = bisect_left(&tg->arctap_fps, &arctap_low_z_fp , arctapfp_compare_fp_asc);
        int arctap_end_index   = bisect_left(&tg->arctap_fps, &arctap_high_z_fp, arctapfp_compare_fp_asc);

        for (int j = arctap_start_index; j < arctap_end_index; j++)
        {
          ArcTapFP *arctap_fp = (ArcTapFP *)list_get(&tg->arctap_fps, j);
          list_push(&arctap_render_list, arctap_fp);
        }
      }

      // Arc heads
      int tg_size = timing_groups->size;
      double curr_fps[tg_size]; float curr_bpms[tg_size]; // won't recommend this
      for (int i = 0; i < tg_size; i++)
      {
        ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, i);
        curr_fps[i]  = get_floor_position(&tg->timing_events, current_ms);
        curr_bpms[i] = get_event_at(&tg->timing_events, current_ms)->bpm;
      }

      for(int i = arc_head_render_list.size - 1; i >= 0; i--)
      {
        ArcSegment *arc_segment = *(ArcSegment **)list_get(&arc_head_render_list, i);
        Arc *arc = arc_segment->arc;
        if (!arc->is_head) continue;
        if (fabs(arc_segment->start_fp - arc->start_fp) > 1e-6) continue;
        double curr_fp = curr_fps[arc->timing_group];
        float curr_bpm = curr_bpms[arc->timing_group];

        int is_void_shader = arc->is_void ? 1 : 0;
        int should_clip_shader = arc->start_timing - current_ms <= 0 ? 1 : 0;
        int negative_bpm_shader = curr_bpm < 0.0f;
        SetShaderValue(render_ctx->arc_shader.shader, render_ctx->arc_shader.isVoid_loc     , &is_void_shader     , SHADER_UNIFORM_INT);
        SetShaderValue(render_ctx->arc_shader.shader, render_ctx->arc_shader.shouldClip_loc , &should_clip_shader , SHADER_UNIFORM_INT);
        SetShaderValue(render_ctx->arc_shader.shader, render_ctx->arc_shader.negativeBPM_loc, &negative_bpm_shader, SHADER_UNIFORM_INT);

        Vector4 tint_low, tint_high;
        tint_low = tint_high = ColorNormalize(color_from_rgba(TRACE_CL)); // Default trace tint
        if (!arc->is_void)
        {
          tint_low  = ColorNormalize(color_from_rgba( arc->color == 0 ? ARC_BLUE_LOW_CL : ARC_PINK_LOW_CL));
          tint_high = ColorNormalize(color_from_rgba( arc->color == 0 ? ARC_BLUE_HIGH_CL : ARC_PINK_HIGH_CL));
        }
        SetShaderValue(render_ctx->arc_shader.shader, render_ctx->arc_shader.tintLow_loc , &tint_low , SHADER_UNIFORM_VEC4);
        SetShaderValue(render_ctx->arc_shader.shader, render_ctx->arc_shader.tintHigh_loc, &tint_high, SHADER_UNIFORM_VEC4);

        float x_pos = arc_x_to_world(arc->x1);
        float y_pos = arc_y_to_world(arc->y1);
        float z_pos = floor_position_to_z(arc->start_fp - curr_fp, base_bpm, scroll_speed);
        float head_scale = arc->is_void ? TRACE_MESH_SCALE : ARC_MESH_SCALE;
        Matrix tr = MatrixMultiply(MatrixScale(head_scale, head_scale, head_scale), MatrixTranslate(x_pos, y_pos, z_pos));
        DrawMesh(arctap_renderer->arc_head.mesh, arctap_renderer->arc_head.material, tr);
      }

      list_sort_by(&arctap_render_list, arctapfp_compare_fp_asc);
      for (int i = arctap_render_list.size - 1; i >= 0; i--)
      {
          ArcTapFP *arctap_fp = (ArcTapFP *)list_get(&arctap_render_list, i);
          ArcTap *arctap = arctap_fp->arctap;
          double curr_fp = curr_fps[arctap->timing_group];

          float z_pos = floor_position_to_z(arctap->fp - curr_fp, base_bpm, scroll_speed);
          arctap_render_test(&arctap_renderer->arctap, arctap, z_pos);
      }
      rlEnableBackfaceCulling();
    rlPopMatrix();
  EndMode3D();
  list_free(&arctap_render_list);
  list_free(&arc_head_render_list);
}
