#include <math.h>
#include "render_service.h"
#include "color_services.h"
#include "constants.h"
#include "data/app_configs/app_config.h"
#include "data/chart_timing_groups/chart_timing_group.h"
#include "data/gameplay_events/gameplay_events.h"
#include "gameplay/arc_formula.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

void render_holds_taps(List *timing_groups, RenderContext *render_ctx, HoldTapRenderer *hold_tap_renderer,
                       float current_ms, float base_bpm, float scroll_speed)
{
  float low_z_clip  = z_to_floor_position(-9.0f, base_bpm, scroll_speed);
  float high_z_clip = z_to_floor_position(-100.0f, base_bpm, scroll_speed);
  BeginTextureMode(hold_tap_renderer->layer);
    ClearBackground(BLANK);
    BeginMode3D(render_ctx->camera);
      rlPushMatrix();
        rlScalef(1.7896f, 1.0f, 1.0f);
        rlDisableBackfaceCulling();
          for (int i = 0; i < timing_groups->size; i++) {
            ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, i);
            float curr_fp  = get_floor_position(&tg->timing_events, current_ms);

            // Holds
            for (int j = 0; j < tg->holds.size; j++)
            {
              Hold *hold    = (Hold *)list_get(&tg->holds, j);
              float z_pos   = floor_position_to_z(hold->start_fp - curr_fp, base_bpm, scroll_speed);
              float z_scale = floor_position_to_z(hold->end_fp - hold->start_fp, base_bpm, scroll_speed);
              if (z_pos < -100.0f || (z_pos > 9.0f && z_scale > 9.0f)) continue;
              float alpha   = hold->start_timing < current_ms && !hold->is_active ? 0.4f : 1.0f;
              hold_render_test(&hold_tap_renderer->hold, hold, z_pos, z_scale, alpha);
            }

            // Taps
            TapFP tap_low_z_fp  = { .fp = curr_fp - low_z_clip };
            TapFP tap_high_z_fp = { .fp = curr_fp + high_z_clip };
            int tap_start_index = bisect_left(&tg->tap_fps, &tap_low_z_fp , tapfp_compare_fp_asc);
            int tap_end_index   = bisect_left(&tg->tap_fps, &tap_high_z_fp, tapfp_compare_fp_asc);
            for (int j = tap_start_index; j < tap_end_index; j++)
            {
              TapFP *tap_fp = (TapFP *)list_get(&tg->tap_fps, j);
              Tap *tap      = tap_fp->tap;
              float diff_fp = tap->fp - curr_fp;
              float z_pos   = floor_position_to_z(diff_fp, base_bpm, scroll_speed);
              float z_scale = Clamp(Lerp(1.8f, 5.8f,
                                         floor_position_to_z(diff_fp, base_bpm, scroll_speed) / -100.0f),
                                    1.8f, 5.8f);
              tap_render_test(&hold_tap_renderer->tap, tap, z_pos, z_scale);

              for (int k = 0; k < tap->connector_x.size; k++)
              {
                float x = *(float *)list_get(&tap->connector_x, k);
                float y = *(float *)list_get(&tap->connector_y, k);
                DrawThickLine3D((Vector3){ lane_to_world_x(tap->lane), 0.0f, z_pos },
                                (Vector3){ x, y, z_pos },
                                Lerp(0.04f, 0.11f, floor_position_to_z(diff_fp, base_bpm, scroll_speed) / -100.0f),
                                render_ctx->chart_settings.skin_side == SK_CONFLICT
                                  ? color_from_rgba(CONFICT_CONNECTOR_CL)
                                  : color_from_rgba(LIGHT_CONNECTOR_CL)
                                );
              }
            }
          }
        rlEnableBackfaceCulling();
      rlPopMatrix();
    EndMode3D();
  EndTextureMode();
}

void render_note_shadows(List *timing_groups, RenderContext *render_ctx, ShadowRenderer *shadow_renderer,
                        float current_ms, float base_bpm, float scroll_speed)
{
  float low_z_clip  = z_to_floor_position(50.0f, base_bpm, scroll_speed);
  float high_z_clip = z_to_floor_position(-100.0f, base_bpm, scroll_speed);
  BeginTextureMode(shadow_renderer->layer);
    ClearBackground(BLANK);
    BeginMode3D(render_ctx->camera);
      rlPushMatrix();
        rlScalef(1.7896f, 1.0f, 1.0f);
        rlDisableBackfaceCulling();
        rlDisableDepthTest();
          for (int i = 0; i < timing_groups->size; i++)
          {
            ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, i);
            float curr_fp = get_floor_position(&tg->timing_events, current_ms);
            float curr_event_bpm = get_event_at(&tg->timing_events, current_ms)->bpm;

            ArcSegment segment_low_fp  = { .start_fp = curr_fp + low_z_clip };
            ArcSegment segment_high_fp = { .start_fp = curr_fp + high_z_clip };
            int segment_start_index = bisect_left(&tg->arc_segments, &segment_low_fp , arc_segment_compare_start_fp_asc);
            int segment_end_index   = bisect_left(&tg->arc_segments, &segment_high_fp, arc_segment_compare_start_fp_asc);

            for (int j = segment_start_index; j < segment_end_index; j++)
            {
              ArcSegment *arc_segment = (ArcSegment *)list_get(&tg->arc_segments, j);
              Arc *arc = arc_segment->arc;

              int is_void_shader = arc->is_void ? 1 : 0;
              int should_clip_shader = arc->start_timing - current_ms <= 0 ? 1 : 0;
              int negative_bpm_shader = curr_event_bpm < 0.0f;
              SetShaderValue(arc_segment->shadow_r.material.shader, render_ctx->arc_clip_shader.isVoid_loc     , &is_void_shader     , SHADER_UNIFORM_INT);
              SetShaderValue(arc_segment->shadow_r.material.shader, render_ctx->arc_clip_shader.shouldClip_loc , &should_clip_shader , SHADER_UNIFORM_INT);
              SetShaderValue(arc_segment->shadow_r.material.shader, render_ctx->arc_clip_shader.negativeBPM_loc, &negative_bpm_shader, SHADER_UNIFORM_INT);

              float z_pos = floor_position_to_z(arc_segment->start_fp - curr_fp, base_bpm, scroll_speed);
              DrawMesh(arc_segment->shadow_r.mesh, arc_segment->shadow_r.material, MatrixTranslate(0.0f, 0.0f, z_pos));
            }

            ArcTapFP arctap_low_fp  = { .fp = curr_fp + low_z_clip };
            ArcTapFP arctap_high_fp = { .fp = curr_fp + high_z_clip };
            int arctap_start_index = bisect_left(&tg->arctap_fps, &arctap_low_fp , arctapfp_compare_fp_asc);
            int arctap_end_index   = bisect_left(&tg->arctap_fps, &arctap_high_fp, arctapfp_compare_fp_asc);

            for (int j = arctap_start_index; j < arctap_end_index; j++)
            {
              ArcTapFP *arctap_fp = (ArcTapFP *)list_get(&tg->arctap_fps, j);
              ArcTap *arctap = arctap_fp->arctap;
              float arctap_z_pos = floor_position_to_z(arctap->fp - curr_fp, base_bpm, scroll_speed);
              Matrix arctap_mt   = MatrixMultiply(MatrixRotateX(-180.0f * DEG2RAD),
                                                  MatrixTranslate(arc_world_x_at(arctap->timing, arctap->arc, arctap->arc->x1),
                                                                  0.0f, arctap_z_pos));
              DrawMesh(shadow_renderer->arctap_shadow.mesh, shadow_renderer->arctap_shadow.material, arctap_mt);
            }
          }
        rlEnableDepthTest();
        rlEnableBackfaceCulling();
      rlPopMatrix();
    EndMode3D();
  EndTextureMode();
}

void render_arcs(List *timing_groups, RenderContext *render_ctx, ArcRenderer *arc_renderer,
                 float current_ms, float base_bpm, float scroll_speed)
{
  float low_z_clip  = z_to_floor_position(50.0f, base_bpm, scroll_speed);
  float high_z_clip = z_to_floor_position(-100.0f, base_bpm, scroll_speed);
  BeginTextureMode(arc_renderer->layer);
    ClearBackground(BLANK);
    BeginMode3D(render_ctx->camera);
      rlPushMatrix();
        rlScalef(1.7896f, 1.0f, 1.0f);
        rlDisableBackfaceCulling();
        BeginBlendMode(BLEND_ALPHA);
        for (int i = 0; i < timing_groups->size; i++)
        {
          ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, i);
          float curr_fp = get_floor_position(&tg->timing_events, current_ms);
          float curr_event_bpm = get_event_at(&tg->timing_events, current_ms)->bpm;

          // grab starting index withing certain +-ms padding
          ArcSegment low_segment  = { .start_fp = curr_fp + low_z_clip };
          ArcSegment high_segment = { .start_fp = curr_fp + high_z_clip };
          int start_index = bisect_left(&tg->arc_segments, &low_segment , arc_segment_compare_start_fp_asc);
          int end_index   = bisect_left(&tg->arc_segments, &high_segment, arc_segment_compare_start_fp_asc);

          for (int j = start_index; j < end_index; j++)
          {
            ArcSegment *arc_segment = (ArcSegment *)list_get(&tg->arc_segments, j);
            Arc *arc = arc_segment->arc;
            float arc_world_x1 = arc_x_to_world(arc->x1);
            float arc_world_y1 = arc_y_to_world(arc->y1);

            int is_void_shader = arc->is_void ? 1 : 0;
            int should_clip_shader = arc->start_timing - current_ms <= 0 ? 1 : 0;
            int negative_bpm_shader = curr_event_bpm < 0.0f;
            SetShaderValue(arc_segment->shadow_r.material.shader, render_ctx->arc_clip_shader.isVoid_loc     , &is_void_shader     , SHADER_UNIFORM_INT);
            SetShaderValue(arc_segment->shadow_r.material.shader, render_ctx->arc_clip_shader.shouldClip_loc , &should_clip_shader , SHADER_UNIFORM_INT);
            SetShaderValue(arc_segment->shadow_r.material.shader, render_ctx->arc_clip_shader.negativeBPM_loc, &negative_bpm_shader, SHADER_UNIFORM_INT);

            float z_pos = floor_position_to_z(arc_segment->start_fp - curr_fp, base_bpm, scroll_speed);
            DrawMesh(arc_segment->mesh_r.mesh, arc_segment->mesh_r.material, MatrixTranslate(0.0f, 0.0f, z_pos));
          }
        }
        EndBlendMode();
        rlEnableBackfaceCulling();
      rlPopMatrix();
    EndMode3D();
  EndTextureMode();
}

// Apparently separate texture fucks a lot of color blending in the process
void render_arc_height_indicators(List *timing_groups, RenderContext *render_ctx, ArcRenderer *arc_renderer,
                                  float current_ms, float base_bpm, float scroll_speed)
{
  float low_z_clip  = z_to_floor_position(50.0f, base_bpm, scroll_speed);
  float high_z_clip = z_to_floor_position(-100.0f, base_bpm, scroll_speed);
  BeginMode3D(render_ctx->camera);
    rlDisableDepthTest();
    rlPushMatrix();
      rlScalef(1.7896f, 1.0f, 1.0f);
      rlDisableBackfaceCulling();
      BeginBlendMode(BLEND_ALPHA);
      for (int i = 0; i < timing_groups->size; i++)
      {
        ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, i);
        float curr_fp = get_floor_position(&tg->timing_events, current_ms);

        ArcSegment low_segment  = { .start_fp = curr_fp + low_z_clip };
        ArcSegment high_segment = { .start_fp = curr_fp + high_z_clip };
        int start_index = bisect_left(&tg->arc_segments, &low_segment , arc_segment_compare_start_fp_asc);
        int end_index   = bisect_left(&tg->arc_segments, &high_segment, arc_segment_compare_start_fp_asc);

        for (int j = start_index; j < end_index; j++)
        {
          ArcSegment *arc_segment = (ArcSegment *)list_get(&tg->arc_segments, j);
          Arc *arc = arc_segment->arc;
          float arc_world_x1 = arc_x_to_world(arc->x1);
          float arc_world_y1 = arc_y_to_world(arc->y1);
          float z_pos = floor_position_to_z(arc_segment->start_fp - curr_fp, base_bpm, scroll_speed);

          if (!arc->is_void && fabsf(arc_segment->start_fp - arc->start_fp) < 1e-6 && (arc->is_head || fabsf(arc->y1 - arc->y2) > 1e-6) )
          {
            rlDisableDepthMask();
            arc_renderer->height_indicator.material.maps->color = arc->color == 0 ? color_from_rgba(ARC_BLUE_LOW_CL)
                                                                                  : color_from_rgba(ARC_PINK_LOW_CL);
            DrawMesh(arc_renderer->height_indicator.mesh, arc_renderer->height_indicator.material,
                     MatrixMultiply(MatrixRotateX(-90.0f * DEG2RAD), MatrixMultiply(MatrixScale(1.0f, arc_world_y1, 1.0f),
                                                                                    MatrixTranslate(arc_world_x1, arc_world_y1 / 2, z_pos))));
            rlEnableDepthMask();
          }
        }
      }
      EndBlendMode();
      rlEnableBackfaceCulling();
    rlPopMatrix();
    rlEnableDepthTest();
  EndMode3D();
}

void render_arctaps(List *timing_groups, RenderContext *render_ctx, ArctapRenderer *arctap_renderer,
                    float current_ms, float base_bpm, float scroll_speed)
{
  float low_z_clip  = z_to_floor_position(-9.0f, base_bpm, scroll_speed);
  float high_z_clip = z_to_floor_position(-100.0f, base_bpm, scroll_speed);
  BeginTextureMode(arctap_renderer->layer);
    ClearBackground(BLANK);
    BeginMode3D(render_ctx->camera);
      rlPushMatrix();
        rlScalef(1.7896f, 1.0f, 1.0f);
        rlDisableBackfaceCulling();
        for (int i = 0; i < timing_groups->size; i++)
        {
          ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, i);
          float curr_fp = get_floor_position(&tg->timing_events, current_ms);

          ArcTapFP arctap_low_z_fp  = { .fp = curr_fp - low_z_clip };
          ArcTapFP arctap_high_z_fp = { .fp = curr_fp + high_z_clip };
          int arctap_start_index = bisect_left(&tg->arctap_fps, &arctap_low_z_fp , arctapfp_compare_fp_asc);
          int arctap_end_index   = bisect_left(&tg->arctap_fps, &arctap_high_z_fp, arctapfp_compare_fp_asc);

          for (int j = arctap_start_index; j < arctap_end_index; j++)
          {
            ArcTapFP *arctap_fp = (ArcTapFP *)list_get(&tg->arctap_fps, j);
            ArcTap *arctap = arctap_fp->arctap;
            float z_pos = floor_position_to_z(arctap->fp - curr_fp, base_bpm, scroll_speed);
            arctap_render_test(&arctap_renderer->arctap, arctap, z_pos);
          }
        }
        rlEnableBackfaceCulling();
      rlPopMatrix();
    EndMode3D();
  EndTextureMode();
}
