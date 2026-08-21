#include <math.h>
#include "render_service.h"
#include "color_services.h"
#include "constants.h"
#include "data/app_configs/app_config.h"
#include "data/chart_timing_groups/chart_timing_group.h"
#include "data/gameplay_events/gameplay_events.h"
#include "gameplay/arc_formula.h"
#include "raymath.h"
#include "rlgl.h"

void chart_reader_render_holds_taps(List *timing_groups, Camera *camera, HoldTapRenderer *hold_tap_renderer,
                                           float current_ms, float base_bpm, float scroll_speed, SkinSide side)
{
  BeginTextureMode(hold_tap_renderer->layer);
    ClearBackground(BLANK);
    BeginMode3D(*camera);
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
            for (int j = 0; j < tg->taps.size; j++)
            {
              Tap *tap      = (Tap *)list_get(&tg->taps, j);
              float diff_fp = tap->fp - curr_fp;
              float z_pos   = floor_position_to_z(diff_fp, base_bpm, scroll_speed);
              if (z_pos < -100.0f || z_pos > 9.0f) continue;
              float z_scale = Clamp(
                Lerp(1.8f, 5.8f, floor_position_to_z(diff_fp, base_bpm, scroll_speed) / -100.0f),
                1.8f, 5.8f
              );
              tap_render_test(&hold_tap_renderer->tap, tap, z_pos, z_scale);

              for (int k = 0; k < tap->connector_x.size; k++)
              {
                float x = *(float *)list_get(&tap->connector_x, k);
                float y = *(float *)list_get(&tap->connector_y, k);
                DrawThickLine3D((Vector3){ lane_to_world_x(tap->lane), 0.0f, z_pos },
                                (Vector3){ x, y, z_pos },
                                Lerp(0.07f, 0.12f, floor_position_to_z(diff_fp, base_bpm, scroll_speed) / -100.0f),
                                side == SK_CONFLICT ? color_from_rgba(CONFICT_CONNECTOR_CL) : color_from_rgba(LIGHT_CONNECTOR_CL));
              }
            }
          }
        rlEnableBackfaceCulling();
      rlPopMatrix();
    EndMode3D();
  EndTextureMode();
}

void chart_reader_render_shadows(List *timing_groups, Camera *camera, ShadowRenderer *shadow_renderer,
                                     float current_ms, float base_bpm, float scroll_speed)
{
  BeginTextureMode(shadow_renderer->layer);
    ClearBackground(BLANK);
    BeginMode3D(*camera);
      rlPushMatrix();
        rlScalef(1.7896f, 1.0f, 1.0f);
        rlDisableBackfaceCulling();
        rlDisableDepthTest();
        BeginBlendMode(BLEND_ALPHA);
        for (int i = 0; i < timing_groups->size; i++)
        {
          ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, i);
          float curr_fp = get_floor_position(&tg->timing_events, current_ms);

          // grab starting index withing certain +-ms padding
          int start_index = 0;
          Arc search_key = { .start_timing = (int)current_ms - ARC_WINDOW_MS_BEHIND };
          start_index = bisect_right(&tg->arcs, &search_key, arc_compare_start_timing_asc);
          start_index = (int)fmaxf(start_index - 1, 0);
          for (int j = start_index; j < tg->arcs.size; j++)
          {
            Arc *arc = (Arc *)list_get(&tg->arcs, j);
            if (arc->start_timing > current_ms + ARC_WINDOW_MS_AHEAD) break;

            float z_pos   = floor_position_to_z(arc->start_fp - curr_fp, base_bpm, scroll_speed);
            float z_scale = floor_position_to_z(arc->end_fp - arc->start_fp , base_bpm, scroll_speed);
            if (z_pos < -100.0f || (z_pos > 9.0f && z_scale > 9.0f)) continue;

            for (int k = 0; k < arc->arctaps.size; k++)
            {
              ArcTap *arctap     = (ArcTap *)list_get(&arc->arctaps, k);
              float arctap_z_pos = floor_position_to_z(arctap->fp - curr_fp, base_bpm, scroll_speed);
              Matrix arctap_mt   = MatrixMultiply(MatrixRotateX(-180.0f * DEG2RAD),
                                                  MatrixTranslate(arc_world_x_at(arctap->timing, arctap->arc, arctap->arc->x1),
                                                                  0.0f, arctap_z_pos));
              DrawMesh(shadow_renderer->arctap_shadow.mesh, shadow_renderer->arctap_shadow.material, arctap_mt);
            }
            DrawMesh(arc->shadow_r.mesh, arc->shadow_r.material, MatrixTranslate(0.0f, 0.0f, z_pos));
          }
        }
        EndBlendMode();
        rlEnableDepthTest();
        rlEnableBackfaceCulling();
      rlPopMatrix();
    EndMode3D();
  EndTextureMode();
}

void chart_reader_render_arcs(List *timing_groups, RenderContext *render_ctx, ArcRenderer *arc_renderer,
                                     float current_ms, float base_bpm, float scroll_speed)
{
  BeginTextureMode(arc_renderer->layer);
    ClearBackground(BLANK);
    BeginMode3D(render_ctx->camera);
      rlPushMatrix();
        rlScalef(1.7896f, 1.0f, 1.0f);
        rlDisableBackfaceCulling();
        for (int i = 0; i < timing_groups->size; i++)
        {
          ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, i);
          float curr_fp = get_floor_position(&tg->timing_events, current_ms);

          // grab starting index withing certain +-ms padding
          int start_index = 0;
          Arc search_key = { .start_timing = (int)current_ms - ARC_WINDOW_MS_BEHIND };
          start_index = bisect_right(&tg->arcs, &search_key, arc_compare_start_timing_asc);
          start_index = (int)fmaxf(start_index - 1, 0);
          for (int j = start_index; j < tg->arcs.size; j++)
          {
            Arc *arc = (Arc *)list_get(&tg->arcs, j);
            if (arc->start_timing > current_ms + ARC_WINDOW_MS_AHEAD) break;

            float z_pos   = floor_position_to_z(arc->start_fp - curr_fp, base_bpm, scroll_speed);
            float z_scale = floor_position_to_z(arc->end_fp - arc->start_fp , base_bpm, scroll_speed);
            if (z_pos < -100.0f || (z_pos > 9.0f && z_scale > 9.0f)) continue;

            // TODO: why the fuck isn't this working
            // float clip_z = 0.0f;
            // SetShaderValue(arc_clip_shader->shader, arc_clip_shader->clipZ_loc, &clip_z, SHADER_UNIFORM_FLOAT);
            // BeginShaderMode(arc_clip_shader->shader);
            DrawMesh(arc->mesh_r.mesh, arc->mesh_r.material, MatrixTranslate(0.0f, 0.0f, z_pos));
          }
        }
        rlEnableBackfaceCulling();
      rlPopMatrix();
    EndMode3D();
  EndTextureMode();
}

void chart_reader_render_arctaps(List *timing_groups, Camera *camera, ArctapRenderer *arctap_renderer,
                                 float current_ms, float base_bpm, float scroll_speed)
{
  BeginTextureMode(arctap_renderer->layer);
    ClearBackground(BLANK);
    BeginMode3D(*camera);
      rlPushMatrix();
        rlScalef(1.7896f, 1.0f, 1.0f);
        rlDisableBackfaceCulling();
        for (int i = 0; i < timing_groups->size; i++)
        {
          ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, i);
          float curr_fp = get_floor_position(&tg->timing_events, current_ms);

          for (int j = 0; j < tg->arctaps.size; j++)
          {
            ArcTap *arctap = (ArcTap *)list_get(&tg->arctaps, j);
            float z_pos = floor_position_to_z(arctap->fp - curr_fp, base_bpm, scroll_speed);
            if (z_pos < -100.0f || z_pos > 9.0f) continue;
            arctap_render_test(&arctap_renderer->arctap, arctap, z_pos);
          }
        }
        rlEnableBackfaceCulling();
      rlPopMatrix();
    EndMode3D();
  EndTextureMode();
}
