#include <math.h>
#include <stdlib.h>
#include "constants.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "render_service.h"
#include "data/app_configs/app_config.h"
#include "data/chart_timing_groups/chart_timing_group.h"
#include "data/custom_types/custom_types.h"
#include "data/gameplay_events/gameplay_events.h"
#include "gameplay/arc_formula.h"

void render_holds_taps(List *timing_groups, NoteRenderLists *note_render_lists, RenderContext *render_ctx, HoldTapRenderer *hold_tap_renderer,
                       float current_ms, float base_bpm, float scroll_speed, double *curr_fps)
{
  List *beatline_list = &note_render_lists->beatline_render_list;
  List *hold_list = &note_render_lists->hold_render_list;
  List *tap_list = &note_render_lists->tap_render_list;

  BeginMode3D(render_ctx->camera);
    rlDisableDepthTest();
    rlDisableBackfaceCulling();
    rlPushMatrix();
      rlScalef(1.7896f, 1.0f, 1.0f);
      BeginBlendMode(BLEND_ALPHA);

        // Beatlines
        for(int i = 0; i < beatline_list->size; i++)
        {
          BeatLine *beatline = (BeatLine *)list_get(beatline_list, i);
          double curr_fp = curr_fps[beatline->timing_group];
          float z_pos    = floor_position_to_z(beatline->fp - curr_fp, base_bpm, scroll_speed);
          DrawBeatline(z_pos, Lerp(beatline->thickness, beatline->thickness * 5, z_pos / -100.0f), beatline->color);
        }
      EndBlendMode();
    rlPopMatrix();
    rlEnableBackfaceCulling();
    rlEnableDepthTest();
  EndMode3D();

  BeginMode3D(render_ctx->camera);
    rlDisableDepthTest();
    rlDisableBackfaceCulling();
    rlPushMatrix();
      rlScalef(1.7896f, 1.0f, 1.0f);
      BeginBlendMode(BLEND_ALPHA);

        // Holds
        for(int i = hold_list->size - 1; i >= 0; i--)
        {
          Hold *hold = *(Hold **)list_get(hold_list, i);
          double curr_fp = curr_fps[hold->timing_group];
          draw_hold(&hold_tap_renderer->hold, hold, current_ms, base_bpm, scroll_speed, curr_fp);
        }

        // Taps
        for(int i = tap_list->size - 1; i >= 0; i--)
        {
          Tap *tap = ((TapFP *)list_get(tap_list, i))->tap;
          double curr_fp = curr_fps[tap->timing_group];
          draw_tap(&hold_tap_renderer->tap, tap, render_ctx, base_bpm, scroll_speed, curr_fp);
        }
      EndBlendMode();
    rlPopMatrix();
    rlEnableBackfaceCulling();
    rlEnableDepthTest();
  EndMode3D();
}

void render_arcs_and_shadows(List *timing_groups, NoteRenderLists *note_render_lists, RenderContext *render_ctx, ArcRenderer *arc_renderer,
                             float current_ms, float base_bpm, float scroll_speed, double *curr_fps, float *curr_bpms)
{
  List *arc_list = &note_render_lists->arc_render_list;
  List *arccap_list = &note_render_lists->arccap_render_list;
  List *arctap_list = &note_render_lists->arctap_render_list;
  BeginMode3D(render_ctx->camera);
    rlDisableDepthTest();
    rlDisableBackfaceCulling();
    rlPushMatrix();
      rlScalef(1.7896f, 1.0f, 1.0f);
      BeginBlendMode(BLEND_ALPHA);

      // Arctap shadows
      for (int i = 0; i < arctap_list->size; i++)
      {
        ArcTapFP *arctap_fp = (ArcTapFP *)list_get(arctap_list, i);
        ArcTap *arctap = arctap_fp->arctap;
        ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, arctap->timing_group);
        if (tg->props.no_shadow) continue;
        double curr_fp = curr_fps[arctap->timing_group];
        float z_pos    = floor_position_to_z(arctap->fp - curr_fp, base_bpm, scroll_speed);
        Matrix tr      = MatrixMultiply(MatrixRotateX(-180.0f * DEG2RAD),
                                        MatrixTranslate(arc_world_x_at(arctap->timing, arctap->arc),
                                                        0.0f, z_pos));
        DrawMesh(arc_renderer->arctap_shadow.mesh, arc_renderer->arctap_shadow.material, tr);
      }

      // Arc/Trace shadows
      for(int i = arc_list->size - 1; i >= 0; i--)
      {
        ArcSegment *arc_segment = *(ArcSegment **)list_get(arc_list, i);
        ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, arc_segment->arc->timing_group);
        if (tg->props.no_shadow) continue;
        double curr_fp =  curr_fps[tg->value];
        float curr_bpm = curr_bpms[tg->value];
        float z_pos    = floor_position_to_z(arc_segment->start_fp - curr_fp, base_bpm, scroll_speed);
        draw_arc_shadow(tg, arc_segment, render_ctx, current_ms, curr_bpm, z_pos);
      }

      // Following arccaps
      for (int i = 0; i < arccap_list->size; i++)
      {
        ArcSegment *arc_segment = *(ArcSegment **)list_get(arccap_list, i);
        Arc *arc = arc_segment->arc;
        ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, arc->timing_group);
        if (tg->props.no_arccap) continue;
        if (!between_int_range_inclusive(current_ms, arc->start_timing, arc->end_timing)) continue;
        draw_arccap(arc_segment, &arc_renderer->arccap.mesh, arc_renderer->arccap.material, 1.0f, ARCCAP_ALPHA, current_ms);
      }

      // Height indicators + Arcs/Traces
      for(int i = arc_list->size - 1; i >= 0; i--)
      {
        ArcSegment *arc_segment = *(ArcSegment **)list_get(arc_list, i);
        ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, arc_segment->arc->timing_group);
        double curr_fp =  curr_fps[tg->value];
        float curr_bpm = curr_bpms[tg->value];
        float z_pos    = floor_position_to_z(arc_segment->start_fp - curr_fp, base_bpm, scroll_speed);
        if (!tg->props.no_height_indicator &&
            !(!tg->props.no_clip && tg->props.no_input && arc_segment->arc->start_timing - current_ms < 0))
          draw_height_indicator(arc_segment, &arc_renderer->height_indicator.mesh, arc_renderer->height_indicator.material, z_pos);
        draw_arc_segment(tg, arc_segment, render_ctx, current_ms, curr_bpm, z_pos);
      }

      // Approaching arccaps
      for(int i = 0; i < arc_list->size; i++)
      {
        ArcSegment *arc_segment = *(ArcSegment **)list_get(arc_list, i);
        Arc *arc = arc_segment->arc;
        ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, arc->timing_group);
        if (tg->props.no_arccap) continue;
        if (arc->is_void) continue;
        if (!arc->is_head) continue;
        if (arc->start_timing - current_ms <= 0) continue;
        if (fabs(arc_segment->start_fp - arc->start_fp) > 1e-6) continue;

        double curr_fp   = curr_fps[tg->value];
        float diff_fp_z  = floor_position_to_z(arc_segment->start_fp - curr_fp, base_bpm, scroll_speed);
        float head_alpha = Clamp(Lerp(ARCCAP_ALPHA, 0.0f, diff_fp_z / -100.0f), 0.0f, ARCCAP_ALPHA);
        float head_scale = Clamp(Lerp(1.0f, ARCCAP_FAR_SCALE, diff_fp_z / -100.0f), 1.0f, ARCCAP_FAR_SCALE);
        draw_arccap(arc_segment, &arc_renderer->arccap.mesh, arc_renderer->arccap.material, head_scale, head_alpha, current_ms);
      }
      EndBlendMode();
    rlPopMatrix();
    rlEnableBackfaceCulling();
    rlEnableDepthTest();
  EndMode3D();
}

void render_arctaps(List *timing_groups, NoteRenderLists *note_render_lists, RenderContext *render_ctx, ArctapRenderer *arctap_renderer,
                    float current_ms, float base_bpm, float scroll_speed, double *curr_fps, float *curr_bpms)
{
  List *arc_list = &note_render_lists->arc_render_list;
  List *arctap_list = &note_render_lists->arctap_render_list;
  BeginMode3D(render_ctx->camera);
    rlDisableBackfaceCulling();
    rlPushMatrix();
      rlScalef(1.7896f, 1.0f, 1.0f);

      // Arc heads
      for(int i = arc_list->size - 1; i >= 0; i--)
      {
        ArcSegment *arc_segment = *(ArcSegment **)list_get(arc_list, i);
        ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, arc_segment->arc->timing_group);
        double curr_fp =  curr_fps[tg->value];
        float curr_bpm = curr_bpms[tg->value];
        draw_arc_head(tg, arc_segment, render_ctx, &arctap_renderer->arc_head,
                      current_ms, curr_bpm, base_bpm, scroll_speed, curr_fp);
      }
      rlEnableDepthTest();

      // Arctaps
      for (int i = arctap_list->size - 1; i >= 0; i--)
      {
        ArcTapFP *arctap_fp = (ArcTapFP *)list_get(arctap_list, i);
        ArcTap *arctap = arctap_fp->arctap;
        double curr_fp = curr_fps[arctap->timing_group];
        float z_pos    = floor_position_to_z(arctap->fp - curr_fp, base_bpm, scroll_speed);
        draw_arctap(&arctap_renderer->arctap, arctap, z_pos);
      }
    rlPopMatrix();
    rlEnableBackfaceCulling();
  EndMode3D();
}
