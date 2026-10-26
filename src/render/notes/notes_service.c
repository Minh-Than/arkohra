#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "constants.h"
#include "notes_service.h"
#include "data/chart_settings/chart_settings.h"
#include "data/chart_timing_groups/chart_timing_group.h"
#include "data/selection/raycast_note.h"
#include "data/gameplay_events/gameplay_events.h"
#include "gameplay/audio_service.h"
#include "gameplay/arc_formula.h"
#include "gameplay/camera/camera_service.h"
#include "render/note_render_lists.h"
#include "render/utils/drawing.h"

NotesService notes_service_init(int glsl, AppConfigs *app_configs)
{
  NotesService service;
  service.tap_tex              = LoadTexture("resources/gameplay/Note/Light/TapNoteLight.png");
  service.hold_tex             = LoadTexture("resources/gameplay/Note/Light/HoldNoteLight.png");
  service.height_indicator_tex = LoadTexture("resources/gameplay/Note/HeightIndicator.png");
  service.arc_tex              = LoadTexture("resources/gameplay/Note/ArcBody.png");
  service.arccap_tex           = LoadTexture("resources/gameplay/Note/ArcCap.png");
  service.arctap_tex           = LoadTexture("resources/gameplay/Note/Light/ArcTapLight.png");
  service.arctap_shadow_tex    = LoadTexture("resources/gameplay/Note/ArcTapShadow.png");

  SetTextureFilter(service.tap_tex, TEXTURE_FILTER_BILINEAR);
  SetTextureFilter(service.hold_tex, TEXTURE_FILTER_BILINEAR);
  SetTextureFilter(service.arctap_tex, TEXTURE_FILTER_BILINEAR);
  SetTextureWrap  (service.hold_tex, TEXTURE_WRAP_CLAMP);
  SetTextureWrap  (service.arc_tex, TEXTURE_WRAP_CLAMP);

  Shader arc_shader = LoadShader(
      TextFormat("resources/shaders/glsl%i/arc_shader.vs", glsl),
      TextFormat("resources/shaders/glsl%i/arc_shader.fs", glsl)
  );
  service.arc_shader = (ArcShader){
    .shader = arc_shader,
    .shouldClip_loc  = GetShaderLocation(arc_shader, "shouldClip"),
    .negativeBPM_loc = GetShaderLocation(arc_shader, "negativeBPM"),
    .tintLow_loc     = GetShaderLocation(arc_shader, "tintLow"),
    .tintHigh_loc    = GetShaderLocation(arc_shader, "tintHigh")
  };

  service.tap              = tap_load_mesh (&service.tap_tex);
  service.hold             = hold_load_mesh(&service.hold_tex);
  service.arc_segment      = arc_segment_generate_mesh(&service.arc_tex, &arc_shader);
  service.arc_shadow       = arc_generate_shadow_mesh(&arc_shader);
  service.arccap           = arc_generate_arccap_mesh(&service.arccap_tex);
  service.height_indicator = arc_generate_arc_height_mesh(&service.height_indicator_tex);
  service.arctap           = arctap_load_mesh(&service.arctap_tex);
  service.arctap_shadow    = arctap_shadow_load_mesh(&service.arctap_shadow_tex);
  service.arc_head         = arc_generate_head_mesh(&service.arc_tex, &arc_shader);

  service.skin_side = SK_LIGHT;
  service.colorblind = app_configs->colorblind;

  return service;
}

static Matrix get_segment_local_transform(ArcSegmentRender *segment_render, bool is_void)
{
  float scale = is_void ? TRACE_MESH_SCALE : ARC_MESH_SCALE;
  Matrix tr = {
    .m0 = scale, .m4 = 0.0f,  .m8  = segment_render->world_x2 - segment_render->world_x1, .m12 = segment_render->world_x1,
    .m1 = 0.0f,  .m5 = scale, .m9  = segment_render->world_y2 - segment_render->world_y1, .m13 = segment_render->world_y1,
    .m2 = 0.0f,  .m6 = 0.0f,  .m10 = (float)segment_render->z_delta                     , .m14 = 0.0f,
    .m3 = 0.0f,  .m7 = 0.0f,  .m11 = 0.0f                                               , .m15 = 1.0f,
  };
  return tr;
}

static Matrix get_segment_shadow_local_transform(ArcSegmentRender *segment_render, bool is_void)
{
  float scale = is_void ? TRACE_MESH_SCALE : ARC_MESH_SCALE;
  Matrix tr = {
    .m0 = scale, .m4 = 0.0f, .m8  = segment_render->world_x2 - segment_render->world_x1, .m12 = segment_render->world_x1,
    .m1 = 0.0f,  .m5 = 1.0f, .m9  = 0.0f                                               , .m13 = 0.0f,
    .m2 = 0.0f,  .m6 = 0.0f, .m10 = (float)segment_render->z_delta                     , .m14 = 0.0f,
    .m3 = 0.0f,  .m7 = 0.0f, .m11 = 0.0f                                               , .m15 = 1.0f,
  };
  return tr;
}

static void process_collision(Ray ray, List *raycast_results, void *note, RawEventType type, Mesh *mesh, Matrix note_tr)
{
  RayCollision collision = GetRayCollisionMesh(ray, *mesh, MatrixMultiply(note_tr, rlGetMatrixTransform()));
  if (!collision.hit) return;
  RaycastNote raycast_note = { .data = note, .collision = collision, .note_type = type };
  list_push(raycast_results, &raycast_note);
}

void notes_service_render(NotesService *notes_service, TrackService *track_service, RenderContext *render_ctx, ChartReader *chart_reader,
                          List *raycast_results)
{
  if (!chart_reader->initialized) return;

  ChartSettings *chart_settings = &render_ctx->chart_settings;
  AudioClock *audio_clock       = &render_ctx->audio_clock;
  List *timing_groups           = &chart_reader->timing_groups;
  NoteRenderLists *render_lists = &chart_reader->render_lists;

  float current_ms   = audio_clock_get_time_ms(audio_clock) - chart_settings->audio_offset;
  float base_bpm     = chart_settings->base_bpm;
  float scroll_speed = chart_settings->scroll_speed;

  // Precalculate tg related values for reuse
  double        curr_fps[timing_groups->size];
  float        curr_bpms[timing_groups->size];
  bool hidegroup_actives[timing_groups->size];
  float  groupalpha_fade[timing_groups->size];
  render_lists_process(render_lists, timing_groups, chart_settings,
                       current_ms, curr_fps, curr_bpms, hidegroup_actives, groupalpha_fade,
                       chart_reader->low_z_clip, chart_reader->high_z_clip);
  List *beatline_list    = &render_lists->beatline_render_list;
  List *hold_list        = &render_lists->hold_render_list;
  List *tap_list         = &render_lists->tap_render_list;
  List *arc_segment_list = &render_lists->arc_segment_render_list;
  List *arccap_list      = &render_lists->arccap_render_list;
  List *arctap_list      = &render_lists->arctap_render_list;

  if (render_ctx->audio_clock.is_playing)
  {
    // Playfield: Track & Single Line Scrolling
    static float scroll_offset = 0.0f;
    float scroll_constant = curr_bpms[0] / base_bpm;
    scroll_offset += GetFrameTime() * render_ctx->chart_settings.scroll_speed * scroll_constant;
    SetShaderValue(track_service->track_shader, track_service->track_scrollOffset_loc, &scroll_offset, SHADER_UNIFORM_FLOAT);
    SetShaderValue(track_service->single_line_shader, track_service->single_line_scrollOffset_loc, &scroll_offset, SHADER_UNIFORM_FLOAT);
  }

  Camera3D final_camera = get_enwidened_camera(render_ctx->camera, &render_lists->enwidencamera_channel, current_ms);
  Ray rayclust = GetScreenToWorldRay(GetMousePosition(), final_camera);
  list_clear(raycast_results);

  // GROUND NOTES
  BeginMode3D(final_camera);
    rlDisableDepthTest();
    rlDisableBackfaceCulling();
    rlPushMatrix();
      rlScalef(FUCKASS_SCALE, 1.0f, 1.0f);
      BeginBlendMode(BLEND_ALPHA);

      // Beatlines
      for(int i = 0; i < beatline_list->size; i++)
      {
        BeatLine *beatline = (BeatLine *)list_get(beatline_list, i);
        double curr_fp = curr_fps[beatline->timing_group];
        double z_pos   = floor_position_to_z(beatline->fp - curr_fp, base_bpm, scroll_speed);
        DrawBeatline(z_pos, Lerp(beatline->thickness, beatline->thickness * 5, z_pos / -100.0f), beatline->color);
      }

      // Holds
      for(int i = hold_list->size - 1; i >= 0; i--)
      {
        HoldRender *hold_render = (HoldRender *)list_get(hold_list, i);
        ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, hold_render->timing_group);
        Hold compare_hold = { .start_fp = hold_render->start_fp, .id = hold_render->hold_id };
        Hold *hold = (Hold *)list_find_sorted(&tg->holds, &compare_hold, hold_compare_start_fp_asc, hold_compare_id_eq);
        if (hold == NULL) continue;

        double curr_fp = curr_fps[tg->value];
        double z_pos   = floor_position_to_z(hold->start_fp - curr_fp, base_bpm, scroll_speed);
        float z_scale  = floor_position_to_z(hold->end_fp - hold->start_fp, base_bpm, scroll_speed);

        Matrix tr = MatrixMultiply(MatrixRotateX(-180.0f * DEG2RAD),
                                   MatrixMultiply(MatrixScale(1.0f, 1.0f, -z_scale),
                                                  MatrixTranslate(lane_to_world_x(hold->lane), 0.0f, z_pos)));
        process_collision(rayclust, raycast_results, hold, HOLD, &notes_service->hold.mesh, tr);

        if (hidegroup_actives[tg->value]) continue;

        MeshRenderable *hold_r = &notes_service->hold;

        float curr_groupalpha = groupalpha_fade[tg->value];
        float alpha = hold->start_timing < current_ms && !hold->is_active ? 0.5f : 1.0f;
        alpha *= curr_groupalpha;
        hold_r->material.maps->color = Fade(hold_r->material.maps->color, alpha);
        DrawMesh(hold_r->mesh, hold_r->material, tr);
      }

      // Taps
      for(int i = tap_list->size - 1; i >= 0; i--)
      {
        TapRender *tap_render = (TapRender *)list_get(tap_list, i);
        ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, tap_render->timing_group);
        Tap compare_tap = { .fp = tap_render->fp, .id = tap_render->tap_id };
        Tap *tap = (Tap *)list_find_sorted(&tg->taps, &compare_tap, tap_compare_fp_asc, tap_compare_id_eq);
        if (tap == NULL) continue;

        double curr_fp = curr_fps[tg->value];
        double diff_fp = tap->fp - curr_fp;
        double z_pos   = floor_position_to_z(diff_fp, base_bpm, scroll_speed);
        float z_scale  = Clamp(Lerp(1.8f, 5.8f, floor_position_to_z(diff_fp, base_bpm, scroll_speed) / -100.0f),
                               1.8f, 5.8f);

        Matrix tr = MatrixMultiply(MatrixRotateX(-180.0f * DEG2RAD),
                                   MatrixMultiply(MatrixScale(1.0f, 1.0f, z_scale),
                                                  MatrixTranslate(lane_to_world_x(tap->lane), 0.0f, z_pos)));
        process_collision(rayclust, raycast_results, tap, TAP, &notes_service->tap.mesh, tr);

        if (hidegroup_actives[tg->value]) continue;

        MeshRenderable *tap_r = &notes_service->tap;

        float alpha = groupalpha_fade[tg->value];
        float fade_ratio = (z_pos - TAP_STOP_FADE) / (TAP_START_FADE - TAP_STOP_FADE);
        tap_r->material.maps[MATERIAL_MAP_DIFFUSE].color = Fade(WHITE, Clamp(fade_ratio, 0.0f, alpha));
        if (tg->props.no_input && tap->timing - current_ms < 0) continue;
        DrawMesh(tap_r->mesh, tap_r->material, tr);

        if (tg->props.no_connection) continue;
        draw_tap_connection(tap, chart_settings, base_bpm, scroll_speed, curr_fp, alpha);
      }
      EndBlendMode();
    rlPopMatrix();
    rlEnableBackfaceCulling();
    rlEnableDepthTest();
  EndMode3D();

  // SKY NOTES
  rlSetClipPlanes(0.01f, 90.0f);
  BeginMode3D(final_camera);
    rlDisableDepthTest();
    rlDisableBackfaceCulling();
    rlPushMatrix();
      rlScalef(FUCKASS_SCALE, 1.0f, 1.0f);
      BeginBlendMode(BLEND_ALPHA);

      // Arctap shadows
      for (int i = 0; i < arctap_list->size; i++)
      {
        ArcTapRender *arctap_render = (ArcTapRender *)list_get(arctap_list, i);
        ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, arctap_render->timing_group);
        ArcTap compare_arctap = { .id = arctap_render->arctap_id, .fp = arctap_render->fp };
        ArcTap *arctap = (ArcTap *)list_find_sorted(&tg->arctaps, &compare_arctap, arctap_compare_fp_asc, arctap_compare_id_eq);
        if (arctap == NULL) continue;

        if (hidegroup_actives[tg->value]) continue;
        if (tg->props.no_shadow) continue;
        if (tg->props.no_input && arctap->timing - current_ms < 0) continue;

        double curr_fp   = curr_fps[tg->value];
        double z_pos     = floor_position_to_z(arctap->fp - curr_fp, base_bpm, scroll_speed);
        float alpha      = groupalpha_fade[tg->value] * arctap_get_shadow_alpha(chart_settings->skin_side);
        float fade_ratio = (z_pos - SKY_STOP_FADE) / (SHADOW_START_FADE - SKY_STOP_FADE);
        notes_service->arctap_shadow.material.maps[MATERIAL_MAP_DIFFUSE].color = Fade(arctap_get_shadow_color(chart_settings->skin_side),
                                                                                      Clamp(fade_ratio, 0.0f, alpha));

        struct Arc find_arc = { .id = arctap->arc_id, .start_timing = arctap->arc_start_timing };
        struct Arc *arc = (struct Arc *)list_find_sorted(&tg->arcs, &find_arc, arc_compare_start_timing_asc, arc_compare_id_eq);
        if (arc == NULL) continue;
        Matrix tr = MatrixMultiply(MatrixRotateX(-180.0f * DEG2RAD),
                                   MatrixTranslate(arc_world_x_at(arctap->timing, arc), 0.0f, z_pos));
        DrawMesh(notes_service->arctap_shadow.mesh, notes_service->arctap_shadow.material, tr);
      }

      // Arc/Trace shadows
      for(int i = arc_segment_list->size - 1; i >= 0; i--)
      {
        ArcSegmentRender *segment_render = (ArcSegmentRender *)list_get(arc_segment_list, i);
        ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, segment_render->timing_group);
        struct Arc find_arc = { .id = segment_render->arc_id, .start_timing = segment_render->arc_start_timing };
        struct Arc *arc = (struct Arc *)list_find_sorted(&tg->arcs, &find_arc, arc_compare_start_timing_asc, arc_compare_id_eq);
        if (arc == NULL) continue;

        if (hidegroup_actives[tg->value]) continue;
        if (tg->props.no_shadow) continue;

        double curr_fp = curr_fps[tg->value];
        float curr_bpm = curr_bpms[tg->value];
        float alpha    = groupalpha_fade[tg->value];
        double z_pos   = floor_position_to_z(segment_render->start_fp - curr_fp, base_bpm, scroll_speed);
        Matrix tr      = MatrixMultiply(get_segment_shadow_local_transform(segment_render, arc->is_void), MatrixTranslate(0.0f, 0.0f, z_pos));
        draw_arc_shadow(tg, arc, &notes_service->arc_shader, &notes_service->arc_shadow, current_ms, curr_bpm, z_pos, alpha, tr);
      }

      // Following arccaps
      for (int i = 0; i < arccap_list->size; i++)
      {
        ArcSegmentRender *segment_render = (ArcSegmentRender *)list_get(arccap_list, i);
        ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, segment_render->timing_group);
        struct Arc find_arc = { .id = segment_render->arc_id, .start_timing = segment_render->arc_start_timing };
        struct Arc *arc = (struct Arc *)list_find_sorted(&tg->arcs, &find_arc, arc_compare_start_timing_asc, arc_compare_id_eq);
        if (arc == NULL) continue;

        if (hidegroup_actives[tg->value]) continue;
        if (tg->props.no_arccap) continue;
        if (!between_int_range_inclusive(current_ms, arc->start_timing, arc->end_timing)) continue;

        float alpha = groupalpha_fade[tg->value] * ARCCAP_ALPHA;
        draw_arccap(segment_render, arc, &notes_service->arccap.mesh, notes_service->arccap.material, 1.0f, alpha, current_ms);
      }

      // Approaching arccaps
      for(int i = 0; i < arc_segment_list->size; i++)
      {
        ArcSegmentRender *segment_render = (ArcSegmentRender *)list_get(arc_segment_list, i);
        ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, segment_render->timing_group);
        struct Arc find_arc = { .id = segment_render->arc_id, .start_timing = segment_render->arc_start_timing };
        struct Arc *arc = (struct Arc *)list_find_sorted(&tg->arcs, &find_arc, arc_compare_start_timing_asc, arc_compare_id_eq);
        if (arc == NULL) continue;

        if (hidegroup_actives[tg->value]) continue;
        if (tg->props.no_arccap) continue;
        if (arc->is_void) continue;
        if (!arc->is_head) continue;
        if (arc->start_timing - current_ms <= 0) continue;
        if (fabs(segment_render->start_fp - arc->start_fp) > 1e-6) continue;

        double curr_fp = curr_fps[tg->value];
        float curr_groupalpha = groupalpha_fade[tg->value];
        float diff_fp_z = floor_position_to_z(segment_render->start_fp - curr_fp, base_bpm, scroll_speed);
        float alpha = Clamp(Lerp(ARCCAP_ALPHA, 0.0f    , diff_fp_z / -100.0f), 0.0f, ARCCAP_ALPHA * curr_groupalpha);
        float scale = Clamp(Lerp(1.0f, ARCCAP_FAR_SCALE, diff_fp_z / -100.0f), 1.0f, ARCCAP_FAR_SCALE);
        draw_arccap(segment_render, arc, &notes_service->arccap.mesh, notes_service->arccap.material, scale, alpha, current_ms);
      }

      // Ending arccaps
      for(int i = 0; i < arc_segment_list->size; i++)
      {
        ArcSegmentRender *segment_render = (ArcSegmentRender *)list_get(arc_segment_list, i);
        ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, segment_render->timing_group);
        struct Arc find_arc = { .id = segment_render->arc_id, .start_timing = segment_render->arc_start_timing };
        struct Arc *arc = (struct Arc *)list_find_sorted(&tg->arcs, &find_arc, arc_compare_start_timing_asc, arc_compare_id_eq);
        if (arc == NULL) continue;

        if (hidegroup_actives[tg->value]) continue;
        if (tg->props.no_arccap) continue;
        if (arc->next_arc != NULL) continue;
        if (fabs(segment_render->end_fp - arc->end_fp) > 1e-6) continue;
        if (abs(arc->end_timing - arc->start_timing) < 2) continue;
        if (arc->end_timing > current_ms) continue;

        double curr_fp = curr_fps[tg->value];
        float alpha = groupalpha_fade[tg->value] * ARCCAP_ALPHA * 0.7f;
        alpha = Clamp(Lerp(ARCCAP_ALPHA, 0.0f, fabsf(current_ms - arc->end_timing) / 120.0f), 0.0f, alpha);
        draw_arccap(segment_render, arc, &notes_service->arccap.mesh, notes_service->arccap.material, 1.0f, alpha, current_ms);
      }

      // Height indicators + Arcs/Traces
      for(int i = arc_segment_list->size - 1; i >= 0; i--)
      {
        ArcSegmentRender *segment_render = (ArcSegmentRender *)list_get(arc_segment_list, i);
        ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, segment_render->timing_group);
        struct Arc find_arc = { .id = segment_render->arc_id, .start_timing = segment_render->arc_start_timing };
        struct Arc *arc = (struct Arc *)list_find_sorted(&tg->arcs, &find_arc, arc_compare_start_timing_asc, arc_compare_id_eq);
        if (arc == NULL) continue;

        double curr_fp = curr_fps[tg->value];
        double z_pos  = floor_position_to_z(segment_render->start_fp - curr_fp, base_bpm, scroll_speed);
        Matrix tr = MatrixMultiply(get_segment_local_transform(segment_render, arc->is_void), MatrixTranslate(0.0f, 0.0f, z_pos));
        process_collision(rayclust, raycast_results, arc, ARC, &notes_service->arc_segment.mesh, tr);

        if (hidegroup_actives[tg->value]) continue;

        float curr_bpm = curr_bpms[tg->value];
        float alpha = groupalpha_fade[tg->value];
        if (!tg->props.no_height_indicator &&
            !(!tg->props.no_clip && tg->props.no_input && arc->start_timing - current_ms < 0))
        {
          draw_height_indicator(segment_render, arc, &notes_service->height_indicator.mesh, notes_service->height_indicator.material,
                                z_pos, alpha, notes_service->colorblind);
        }
        if (!tg->props.no_clip && arc->is_void && arc->end_timing < current_ms) continue;
        draw_arc_segment(tg, arc, &notes_service->arc_shader, &notes_service->arc_segment,
                         current_ms, curr_bpm, z_pos, alpha, notes_service->colorblind, tr);
      }

      // Arc heads
      for(int i = arc_segment_list->size - 1; i >= 0; i--)
      {
        ArcSegmentRender *segment_render = (ArcSegmentRender *)list_get(arc_segment_list, i);
        ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, segment_render->timing_group);
        struct Arc find_arc = { .id = segment_render->arc_id, .start_timing = segment_render->arc_start_timing };
        struct Arc *arc = (struct Arc *)list_find_sorted(&tg->arcs, &find_arc, arc_compare_start_timing_asc, arc_compare_id_eq);
        if (arc == NULL) continue;

        if (hidegroup_actives[tg->value]) continue;
        if (tg->props.no_head) continue;
        if (!arc->is_head) continue;
        if (fabs(segment_render->start_fp - arc->start_fp) > 1e-6) continue;
        if (tg->props.no_input && arc->start_timing - current_ms < 0) continue;

        double curr_fp =  curr_fps[tg->value];
        float curr_bpm = curr_bpms[tg->value];
        float alpha = groupalpha_fade[tg->value];
        draw_arc_head(tg, arc, &notes_service->arc_shader, &notes_service->arc_head,
                      current_ms, curr_bpm, base_bpm, scroll_speed, curr_fp, alpha, notes_service->colorblind);
      }

      // Arctaps
      for (int i = arctap_list->size - 1; i >= 0; i--)
      {
        ArcTapRender *arctap_render = (ArcTapRender *)list_get(arctap_list, i);
        ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, arctap_render->timing_group);
        ArcTap find_arctap = { .id = arctap_render->arctap_id, .fp = arctap_render->fp };
        ArcTap *arctap = (ArcTap *)list_find_sorted(&tg->arctaps, &find_arctap, arctap_compare_fp_asc, arctap_compare_id_eq);
        if (arctap == NULL) continue;

        struct Arc find_arc = { .id = arctap->arc_id, .start_timing = arctap->arc_start_timing };
        struct Arc *arc = (struct Arc *)list_find_sorted(&tg->arcs, &find_arc, arc_compare_start_timing_asc, arc_compare_id_eq); 
        if (arc == NULL) continue;
        double curr_fp = curr_fps[arctap->timing_group];
        double z_pos   = floor_position_to_z(arctap->fp - curr_fp, base_bpm, scroll_speed);
        Matrix tr = MatrixTranslate(arc_world_x_at(arctap->timing, arc),
                                    arc_world_y_at(arctap->timing, arc),
                                    z_pos);
        process_collision(rayclust, raycast_results, arctap, ARCTAP, &notes_service->arctap.mesh, tr);

        if (hidegroup_actives[tg->value]) continue;
        if (tg->props.no_input && arctap->timing - current_ms < 0) continue;

        MeshRenderable *arctap_r = &notes_service->arctap;

        float alpha = groupalpha_fade[tg->value];
        float fade_ratio = (z_pos - SKY_STOP_FADE) / (ARCTAP_START_FADE - SKY_STOP_FADE);
        arctap_r->material.maps[MATERIAL_MAP_DIFFUSE].color = Fade(WHITE, Clamp(fade_ratio, 0.0f, alpha));
        DrawMesh(arctap_r->mesh, arctap_r->material, tr);
      }
      EndBlendMode();
    rlPopMatrix();
    rlEnableBackfaceCulling();
    rlEnableDepthTest();
  EndMode3D();
  rlSetClipPlanes(0.01f, 100.0f);

  if (raycast_results->size > 1)
    list_remove_dups(raycast_results, raycast_note_compare_distance_asc, raycast_note_compare_data);
}

void notes_service_apply_chart(NotesService *notes_service, ChartSettings *chart_settings)
{
  if(notes_service->skin_side == chart_settings->skin_side) return;

  notes_service->skin_side = chart_settings->skin_side;

  UnloadTexture(notes_service->tap_tex);
  notes_service->tap_tex = skin_side_get_tap(chart_settings->skin_side);
  SetTextureFilter(notes_service->tap_tex, TEXTURE_FILTER_BILINEAR);
  notes_service->tap.material.maps[MATERIAL_MAP_DIFFUSE].texture = notes_service->tap_tex;

  UnloadTexture(notes_service->hold_tex);
  notes_service->hold_tex = skin_side_get_hold(chart_settings->skin_side);
  SetTextureFilter(notes_service->hold_tex, TEXTURE_FILTER_BILINEAR);
  notes_service->hold.material.maps[MATERIAL_MAP_DIFFUSE].texture = notes_service->hold_tex;

  UnloadTexture(notes_service->arctap_tex);
  notes_service->arctap_tex = skin_side_get_arctap(chart_settings->skin_side);
  SetTextureFilter(notes_service->arctap_tex, TEXTURE_FILTER_BILINEAR);
  notes_service->arctap.material.maps[MATERIAL_MAP_DIFFUSE].texture = notes_service->arctap_tex;
}

void notes_service_unload(NotesService *notes_service)
{
  renderable_unload(&notes_service->tap);
  renderable_unload(&notes_service->hold);
  renderable_unload(&notes_service->arc_segment);
  renderable_unload(&notes_service->arc_shadow);
  renderable_unload(&notes_service->arccap);
  renderable_unload(&notes_service->arc_head);
  renderable_unload(&notes_service->height_indicator);
  renderable_unload(&notes_service->arctap);
  renderable_unload(&notes_service->arctap_shadow);

  UnloadTexture(notes_service->tap_tex);
  UnloadTexture(notes_service->hold_tex);
  UnloadTexture(notes_service->height_indicator_tex);
  UnloadTexture(notes_service->arc_tex);
  UnloadTexture(notes_service->arccap_tex);
  UnloadTexture(notes_service->arctap_tex);
  UnloadTexture(notes_service->arctap_shadow_tex);

  UnloadShader(notes_service->arc_shader.shader);
}
