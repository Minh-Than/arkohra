#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "constants.h"
#include "color_services.h"
#include "notes_service.h"
#include "data/chart_settings/chart_settings.h"
#include "data/chart_timing_groups/chart_timing_group.h"
#include "data/gameplay_events/gameplay_events.h"
#include "data/keyframe/value_channel.h"
#include "gameplay/audio_service.h"
#include "gameplay/arc_formula.h"
#include "gameplay/camera/camera_service.h"
#include "render/note_render_lists.h"
#include "render/utils/drawing.h"

NotesService notes_service_init(int glsl)
{
  NotesService service;
  service.tap_tex              = LoadTexture("resources/gameplay/Note/Light/TapNoteLight.png");
  service.hold_tex             = LoadTexture("resources/gameplay/Note/Light/HoldNoteLight.png");
  service.height_indicator_tex = LoadTexture("resources/gameplay/Note/HeightIndicator.png");
  service.arc_tex              = LoadTexture("resources/gameplay/Note/ArcBody.png");
  service.arccap_tex           = LoadTexture("resources/gameplay/Note/ArcCap.png");
  service.arctap_tex           = LoadTexture("resources/gameplay/Note/Light/ArcTapLight.png");
  service.arctap_shadow_tex    = LoadTexture("resources/gameplay/Note/ArcTapShadow.png");

  SetTextureWrap  (service.hold_tex, TEXTURE_WRAP_CLAMP);
  SetTextureWrap  (service.arc_tex, TEXTURE_WRAP_CLAMP);

  service.tap              = tap_load_mesh (&service.tap_tex);
  service.hold             = hold_load_mesh(&service.hold_tex);
  service.arccap           = generate_arccap_mesh(&service.arccap_tex);
  service.height_indicator = generate_arc_height_mesh(&service.height_indicator_tex);
  service.arctap           = arctap_load_mesh(&service.arctap_tex);
  service.arctap_shadow    = arctap_shadow_load_mesh(&service.arctap_shadow_tex);

  Shader arc_shader = LoadShader(
      TextFormat("resources/shaders/glsl%i/arc_shader.vs", glsl),
      TextFormat("resources/shaders/glsl%i/arc_shader.fs", glsl)
  );
  service.arc_shader = (ArcShader){
    .shader = arc_shader,
    .shouldClip_loc = GetShaderLocation(arc_shader, "shouldClip"),
    .negativeBPM_loc = GetShaderLocation(arc_shader, "negativeBPM"),
    .tintLow_loc = GetShaderLocation(arc_shader, "tintLow"),
    .tintHigh_loc = GetShaderLocation(arc_shader, "tintHigh")
  };
  service.arc_head = generate_arc_head_mesh(&service.arc_tex, &arc_shader);

  return service;
}

void notes_service_render(NotesService *notes_service, TrackService *track_service, RenderContext *render_ctx, ChartReader *chart_reader)
{
  List *timing_groups = &chart_reader->timing_groups;
  NoteRenderLists *render_lists  = &chart_reader->render_lists;
  List *beatline_list = &render_lists->beatline_render_list;
  List *hold_list     = &render_lists->hold_render_list;
  List *tap_list      = &render_lists->tap_render_list;
  List *arc_list      = &render_lists->arc_render_list;
  List *arccap_list   = &render_lists->arccap_render_list;
  List *arctap_list   = &render_lists->arctap_render_list;

  ChartSettings *chart_settings = &render_ctx->chart_settings;
  AudioClock *audio_clock = &render_ctx->audio_clock;
  float current_ms = audio_clock_get_time_ms(audio_clock) - chart_settings->audio_offset;
  float base_bpm     = chart_settings->base_bpm;
  float scroll_speed = chart_settings->scroll_speed;

  // Precalculate tg related values for reuse
  double        curr_fps[timing_groups->size];
  float        curr_bpms[timing_groups->size];
  bool hidegroup_actives[timing_groups->size];
  float  groupalpha_fade[timing_groups->size];
  process_note_render_lists(chart_reader, chart_settings, current_ms, curr_fps, curr_bpms);
  for (int i = 0; i < timing_groups->size; i++)
  {
    ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, i);
    tg->hidegroup_channel.current_value = value_channel_interpolate(&tg->hidegroup_channel, current_ms);
    hidegroup_actives[i] = fabsf(tg->hidegroup_channel.current_value) > 1e-6;
    tg->groupalpha_channel.current_value = value_channel_interpolate(&tg->groupalpha_channel, current_ms);
    groupalpha_fade[i] = tg->groupalpha_channel.current_value / 255.0f;
  }

  if (render_ctx->audio_clock.is_playing)
  {
    // Playfield: Track & Single Line Scrolling
    static float scroll_offset = 0.0f;
    float scroll_constant = curr_bpms[0] / base_bpm;
    scroll_offset += GetFrameTime() * render_ctx->chart_settings.scroll_speed * scroll_constant;
    SetShaderValue(track_service->scroll_offset_shader, track_service->scrollOffset_loc, &scroll_offset, SHADER_UNIFORM_FLOAT);
  }

  // GROUND NOTES
  Camera3D final_camera = get_enwidened_camera(render_ctx->camera, &render_lists->enwidencamera_channel, current_ms);
  BeginMode3D(final_camera);
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
        double z_pos   = floor_position_to_z(beatline->fp - curr_fp, base_bpm, scroll_speed);
        DrawBeatline(z_pos, Lerp(beatline->thickness, beatline->thickness * 5, z_pos / -100.0f), beatline->color);
      }

      // Holds
      for(int i = hold_list->size - 1; i >= 0; i--)
      {
        Hold *hold = *(Hold **)list_get(hold_list, i);
        ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, hold->timing_group);
        if (hidegroup_actives[tg->value]) continue;
        double curr_fp = curr_fps[tg->value];
        float curr_groupalpha = groupalpha_fade[tg->value];
        draw_hold(&notes_service->hold, hold, current_ms, base_bpm, scroll_speed, curr_fp, curr_groupalpha);
      }

      // Taps
      for(int i = tap_list->size - 1; i >= 0; i--)
      {
        Tap *tap = ((TapFP *)list_get(tap_list, i))->tap;
        ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, tap->timing_group);
        if (hidegroup_actives[tg->value]) continue;
        if (tg->props.no_input && tap->timing - current_ms < 0) continue;
        double curr_fp = curr_fps[tg->value];
        float curr_groupalpha = groupalpha_fade[tg->value];
        draw_tap(&notes_service->tap, tap, base_bpm, scroll_speed, curr_fp, curr_groupalpha);
        if (tg->props.no_connection) continue;
        draw_tap_connection(tap, chart_settings, base_bpm, scroll_speed, curr_fp, curr_groupalpha);
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
      rlScalef(1.7896f, 1.0f, 1.0f);
      BeginBlendMode(BLEND_ALPHA);
      // Arctap shadows
      for (int i = 0; i < arctap_list->size; i++)
      {
        ArcTapFP *arctap_fp = (ArcTapFP *)list_get(arctap_list, i);
        ArcTap *arctap = arctap_fp->arctap;
        ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, arctap->timing_group);
        if (hidegroup_actives[tg->value]) continue;
        if (tg->props.no_shadow) continue;
        if (tg->props.no_input && arctap->timing - current_ms < 0) continue;
        double curr_fp   = curr_fps[tg->value];
        float curr_groupalpha = groupalpha_fade[tg->value];
        double z_pos     = floor_position_to_z(arctap->fp - curr_fp, base_bpm, scroll_speed);
        float fade_ratio = (z_pos - SKY_STOP_FADE) / (SHADOW_START_FADE - SKY_STOP_FADE);
        notes_service->arctap_shadow.material.maps[MATERIAL_MAP_DIFFUSE].color = Fade(color_from_rgba(NOTE_SHADOW_CL),
                                                                                      Clamp(fade_ratio, 0.0f, ARCTAP_SHADOW_ALPHA * curr_groupalpha));
        Matrix tr = MatrixMultiply(MatrixRotateX(-180.0f * DEG2RAD),
                                        MatrixTranslate(arc_world_x_at(arctap->timing, arctap->arc),
                                                        0.0f, z_pos));
        DrawMesh(notes_service->arctap_shadow.mesh, notes_service->arctap_shadow.material, tr);
      }

      // Arc/Trace shadows
      for(int i = arc_list->size - 1; i >= 0; i--)
      {
        ArcSegment *arc_segment = *(ArcSegment **)list_get(arc_list, i);
        ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, arc_segment->arc->timing_group);
        if (hidegroup_actives[tg->value]) continue;
        if (tg->props.no_shadow) continue;
        double curr_fp =  curr_fps[tg->value];
        float curr_bpm = curr_bpms[tg->value];
        double z_pos   = floor_position_to_z(arc_segment->start_fp - curr_fp, base_bpm, scroll_speed);
        float curr_groupalpha = groupalpha_fade[tg->value];
        draw_arc_shadow(tg, arc_segment, &notes_service->arc_shader, current_ms, curr_bpm, z_pos, curr_groupalpha);
      }

      // Following arccaps
      for (int i = 0; i < arccap_list->size; i++)
      {
        ArcSegment *arc_segment = *(ArcSegment **)list_get(arccap_list, i);
        struct Arc *arc = arc_segment->arc;
        ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, arc->timing_group);
        if (hidegroup_actives[tg->value]) continue;
        if (tg->props.no_arccap) continue;

        float curr_groupalpha = groupalpha_fade[tg->value];
        if (!between_int_range_inclusive(current_ms, arc->start_timing, arc->end_timing)) continue;
        draw_arccap(arc_segment, &notes_service->arccap.mesh, notes_service->arccap.material, 1.0f, ARCCAP_ALPHA * curr_groupalpha, current_ms);
      }

      // Approaching arccaps
      for(int i = 0; i < arc_list->size; i++)
      {
        ArcSegment *arc_segment = *(ArcSegment **)list_get(arc_list, i);
        struct Arc *arc = arc_segment->arc;
        ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, arc->timing_group);
        if (hidegroup_actives[tg->value]) continue;
        if (tg->props.no_arccap) continue;
        if (arc->is_void) continue;
        if (!arc->is_head) continue;
        if (arc->start_timing - current_ms <= 0) continue;
        if (fabs(arc_segment->start_fp - arc->start_fp) > 1e-6) continue;

        double curr_fp  = curr_fps[tg->value];
        float curr_groupalpha = groupalpha_fade[tg->value];
        float diff_fp_z = floor_position_to_z(arc_segment->start_fp - curr_fp, base_bpm, scroll_speed);
        float cap_alpha = Clamp(Lerp(ARCCAP_ALPHA, 0.0f    , diff_fp_z / -100.0f), 0.0f, ARCCAP_ALPHA * curr_groupalpha);
        float cap_scale = Clamp(Lerp(1.0f, ARCCAP_FAR_SCALE, diff_fp_z / -100.0f), 1.0f, ARCCAP_FAR_SCALE);
        draw_arccap(arc_segment, &notes_service->arccap.mesh, notes_service->arccap.material, cap_scale, cap_alpha, current_ms);
      }

      // Ending arccaps
      for(int i = 0; i < arc_list->size; i++)
      {
        ArcSegment *arc_segment = *(ArcSegment **)list_get(arc_list, i);
        struct Arc *arc = arc_segment->arc;
        ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, arc->timing_group);
        if (hidegroup_actives[tg->value]) continue;
        if (tg->props.no_arccap) continue;
        if (arc->next_arc != NULL) continue;
        if (fabs(arc_segment->end_fp - arc->end_fp) > 1e-6) continue;
        if (abs(arc->end_timing - arc->start_timing) < 2) continue;
        if (arc->end_timing > current_ms) continue;

        double curr_fp  = curr_fps[tg->value];
        float curr_groupalpha = groupalpha_fade[tg->value];
        float cap_alpha = Clamp(Lerp(ARCCAP_ALPHA, 0.0f, fabsf(current_ms - arc->end_timing) / 120.0f), 0.0f, ARCCAP_ALPHA * 0.7f * curr_groupalpha);
        draw_arccap(arc_segment, &notes_service->arccap.mesh, notes_service->arccap.material, 1.0f, cap_alpha, current_ms);
      }

      // Height indicators + Arcs/Traces
      for(int i = arc_list->size - 1; i >= 0; i--)
      {
        ArcSegment *arc_segment = *(ArcSegment **)list_get(arc_list, i);
        ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, arc_segment->arc->timing_group);
        if (hidegroup_actives[tg->value]) continue;
        double curr_fp =  curr_fps[tg->value];
        float curr_bpm = curr_bpms[tg->value];
        float curr_groupalpha = groupalpha_fade[tg->value];
        double z_pos  = floor_position_to_z(arc_segment->start_fp - curr_fp, base_bpm, scroll_speed);
        if (!tg->props.no_height_indicator &&
            !(!tg->props.no_clip && tg->props.no_input && arc_segment->arc->start_timing - current_ms < 0))
          draw_height_indicator(arc_segment, &notes_service->height_indicator.mesh, notes_service->height_indicator.material, z_pos, curr_groupalpha);
        struct Arc *arc = arc_segment->arc;
        if (!tg->props.no_clip && arc->is_void && arc->end_timing < current_ms) continue;
        draw_arc_segment(tg, arc_segment, &notes_service->arc_shader, current_ms, curr_bpm, z_pos, curr_groupalpha);
      }

      // Arc heads
      for(int i = arc_list->size - 1; i >= 0; i--)
      {
        ArcSegment *arc_segment = *(ArcSegment **)list_get(arc_list, i);
        struct Arc *arc = arc_segment->arc;
        ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, arc_segment->arc->timing_group);
        if (hidegroup_actives[tg->value]) continue;
        if (tg->props.no_input && arc->start_timing - current_ms < 0) continue;
        double curr_fp =  curr_fps[tg->value];
        float curr_bpm = curr_bpms[tg->value];
        float curr_groupalpha = groupalpha_fade[tg->value];
        draw_arc_head(tg, arc_segment, &notes_service->arc_shader, &notes_service->arc_head,
                      current_ms, curr_bpm, base_bpm, scroll_speed, curr_fp, curr_groupalpha);
      }

      // Arctaps
      for (int i = arctap_list->size - 1; i >= 0; i--)
      {
        ArcTapFP *arctap_fp = (ArcTapFP *)list_get(arctap_list, i);
        ArcTap *arctap = arctap_fp->arctap;
        ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, arctap->timing_group);
        if (hidegroup_actives[tg->value]) continue;
        if (tg->props.no_input && arctap->timing - current_ms < 0) continue;
        double curr_fp = curr_fps[arctap->timing_group];
        float curr_groupalpha = groupalpha_fade[tg->value];
        double z_pos   = floor_position_to_z(arctap->fp - curr_fp, base_bpm, scroll_speed);
        draw_arctap(&notes_service->arctap, arctap, z_pos, curr_groupalpha);
      }
      EndBlendMode();
    rlPopMatrix();
    rlEnableBackfaceCulling();
    rlEnableDepthTest();
  EndMode3D();
  rlSetClipPlanes(0.01f, 100.0f);
}

void notes_service_apply_chart(NotesService *notes_service, ChartSettings *chart_settings)
{
  UnloadTexture(notes_service->hold_tex);
  notes_service->hold_tex = skin_side_get_hold(chart_settings->skin_side);
  SetTextureFilter(notes_service->hold_tex, TEXTURE_FILTER_BILINEAR);

  UnloadTexture(notes_service->tap_tex);
  notes_service->tap_tex = skin_side_get_tap(chart_settings->skin_side);
  SetTextureFilter(notes_service->tap_tex, TEXTURE_FILTER_BILINEAR);

  UnloadTexture(notes_service->arctap_tex);
  notes_service->arctap_tex = skin_side_get_arctap(chart_settings->skin_side);
  SetTextureFilter(notes_service->arctap_tex, TEXTURE_FILTER_BILINEAR);
}

void notes_service_unload(NotesService *notes_service)
{
  renderable_unload(&notes_service->tap);
  renderable_unload(&notes_service->hold);
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
