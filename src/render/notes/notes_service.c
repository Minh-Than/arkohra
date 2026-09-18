#include "constants.h"
#include "data/chart_timing_groups/chart_timing_group.h"
#include "data/keyframe/value_channel.h"
#include "raylib.h"
#include "raymath.h"
#include "render/utils/drawing.h"
#include "rlgl.h"
#include "notes_service.h"
#include "data/gameplay_events/gameplay_events.h"
#include "gameplay/arc_formula.h"

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

void notes_service_render(NotesService *notes_service, ChartSettings *chart_settings,
                          Camera3D camera, ChartReader *chart_reader, float current_ms)
{
  List *timing_groups = &chart_reader->timing_groups;
  NoteRenderLists *note_render_lists = &chart_reader->render_lists;
  List *beatline_list = &note_render_lists->beatline_render_list;
  List *hold_list     = &note_render_lists->hold_render_list;
  List *tap_list      = &note_render_lists->tap_render_list;
  List *arc_list      = &note_render_lists->arc_render_list;
  List *arccap_list   = &note_render_lists->arccap_render_list;
  List *arctap_list   = &note_render_lists->arctap_render_list;

  float base_bpm = chart_settings->base_bpm;
  float scroll_speed = chart_settings->scroll_speed;

  // Precalculate tg related values for reuse
  double curr_fps[chart_reader->timing_groups.size];
  float curr_bpms[chart_reader->timing_groups.size];
  bool hidegroup_actives[chart_reader->timing_groups.size];
  process_note_render_lists(chart_reader, chart_settings, current_ms, curr_fps, curr_bpms);
  for (int i = 0; i < timing_groups->size; i++)
  {
    ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, i);
    tg->hidegroup_channel.current_value = value_channel_interpolate(&tg->hidegroup_channel, current_ms);
    hidegroup_actives[i] = fabsf(tg->hidegroup_channel.current_value) > 1e-6;
  }

  // Beatlines
  BeginMode3D(camera);
    rlDisableDepthTest();
    rlDisableBackfaceCulling();
    rlPushMatrix();
      rlScalef(1.7896f, 1.0f, 1.0f);
      BeginBlendMode(BLEND_ALPHA);

        for(int i = 0; i < beatline_list->size; i++)
        {
          BeatLine *beatline = (BeatLine *)list_get(beatline_list, i);
          double curr_fp = curr_fps[beatline->timing_group];
          double z_pos    = floor_position_to_z(beatline->fp - curr_fp, base_bpm, scroll_speed);
          DrawBeatline(z_pos, Lerp(beatline->thickness, beatline->thickness * 5, z_pos / -100.0f), beatline->color);
        }
      EndBlendMode();
    rlPopMatrix();
    rlEnableBackfaceCulling();
    rlEnableDepthTest();
  EndMode3D();

  // Holds + Taps
  BeginMode3D(camera);
    rlDisableDepthTest();
    rlDisableBackfaceCulling();
    rlPushMatrix();
      rlScalef(1.7896f, 1.0f, 1.0f);
      BeginBlendMode(BLEND_ALPHA);

        // Holds
        for(int i = hold_list->size - 1; i >= 0; i--)
        {
          Hold *hold = *(Hold **)list_get(hold_list, i);
          ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, hold->timing_group);
          if (hidegroup_actives[tg->value]) continue;
          double curr_fp = curr_fps[hold->timing_group];
          draw_hold(&notes_service->hold, hold, current_ms, base_bpm, scroll_speed, curr_fp);
        }

        // Taps
        for(int i = tap_list->size - 1; i >= 0; i--)
        {
          Tap *tap = ((TapFP *)list_get(tap_list, i))->tap;
          ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, tap->timing_group);
          if (hidegroup_actives[tg->value]) continue;
          if (tg->props.no_input && tap->timing - current_ms < 0) continue;
          double curr_fp = curr_fps[tap->timing_group];
          draw_tap(&notes_service->tap, tap, chart_settings, base_bpm, scroll_speed, curr_fp);
        }
      EndBlendMode();
    rlPopMatrix();
    rlEnableBackfaceCulling();
    rlEnableDepthTest();
  EndMode3D();

  // Arcs + shadows + arccaps + height_indicator
  rlSetClipPlanes(0.01f, 90.0f);
  BeginMode3D(camera);
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
        double curr_fp = curr_fps[arctap->timing_group];
        double z_pos    = floor_position_to_z(arctap->fp - curr_fp, base_bpm, scroll_speed);
        float fade_ratio = (z_pos - SKY_STOP_FADE) / (SHADOW_START_FADE - SKY_STOP_FADE);
        notes_service->arctap_shadow.material.maps[MATERIAL_MAP_DIFFUSE].color = Fade((Color){ 90, 90, 90, 255 }, Clamp(fade_ratio, 0.0f, 0.25f));
        Matrix tr      = MatrixMultiply(MatrixRotateX(-180.0f * DEG2RAD),
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
        double z_pos    = floor_position_to_z(arc_segment->start_fp - curr_fp, base_bpm, scroll_speed);
        draw_arc_shadow(tg, arc_segment, &notes_service->arc_shader, current_ms, curr_bpm, z_pos);
      }

      // Following arccaps
      for (int i = 0; i < arccap_list->size; i++)
      {
        ArcSegment *arc_segment = *(ArcSegment **)list_get(arccap_list, i);
        struct Arc *arc = arc_segment->arc;
        ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, arc->timing_group);
        if (hidegroup_actives[tg->value]) continue;
        if (tg->props.no_arccap) continue;
        if (!between_int_range_inclusive(current_ms, arc->start_timing, arc->end_timing)) continue;
        draw_arccap(arc_segment, &notes_service->arccap.mesh, notes_service->arccap.material, 1.0f, ARCCAP_ALPHA, current_ms);
      }

      // Height indicators + Arcs/Traces
      for(int i = arc_list->size - 1; i >= 0; i--)
      {
        ArcSegment *arc_segment = *(ArcSegment **)list_get(arc_list, i);
        ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, arc_segment->arc->timing_group);
        if (hidegroup_actives[tg->value]) continue;
        double curr_fp =  curr_fps[tg->value];
        float curr_bpm = curr_bpms[tg->value];
        double z_pos    = floor_position_to_z(arc_segment->start_fp - curr_fp, base_bpm, scroll_speed);
        if (!tg->props.no_height_indicator &&
            !(!tg->props.no_clip && tg->props.no_input && arc_segment->arc->start_timing - current_ms < 0))
          draw_height_indicator(arc_segment, &notes_service->height_indicator.mesh, notes_service->height_indicator.material, z_pos);
        draw_arc_segment(tg, arc_segment, &notes_service->arc_shader, current_ms, curr_bpm, z_pos);
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
        float diff_fp_z = floor_position_to_z(arc_segment->start_fp - curr_fp, base_bpm, scroll_speed);
        float cap_alpha = Clamp(Lerp(ARCCAP_ALPHA, 0.0f    , diff_fp_z / -100.0f), 0.0f, ARCCAP_ALPHA);
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
        if (arc->prev_arc != NULL) continue;
        if (abs(arc->end_timing - arc->start_timing) < 2) continue;
        if (arc->end_timing > current_ms) continue;

        double curr_fp  = curr_fps[tg->value];
        float cap_alpha = Clamp(Lerp(ARCCAP_ALPHA, 0.0f, fabsf(current_ms - arc->end_timing) / 120.0f), 0.0f, ARCCAP_ALPHA);
        draw_arccap(arc_segment, &notes_service->arccap.mesh, notes_service->arccap.material, 1.0f, cap_alpha, current_ms);
      }
      EndBlendMode();
    rlPopMatrix();
    rlEnableBackfaceCulling();
    rlEnableDepthTest();
  EndMode3D();

  // Arc heads + arctaps
  BeginMode3D(camera);
    rlDisableDepthTest();
    rlDisableBackfaceCulling();
    rlPushMatrix();
      rlScalef(1.7896f, 1.0f, 1.0f);

      for(int i = arc_list->size - 1; i >= 0; i--)
      {
        ArcSegment *arc_segment = *(ArcSegment **)list_get(arc_list, i);
        struct Arc *arc = arc_segment->arc;
        ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, arc_segment->arc->timing_group);
        if (hidegroup_actives[tg->value]) continue;
        if (tg->props.no_input && arc->start_timing - current_ms < 0) continue;
        double curr_fp =  curr_fps[tg->value];
        float curr_bpm = curr_bpms[tg->value];
        draw_arc_head(tg, arc_segment, &notes_service->arc_shader, &notes_service->arc_head,
                      current_ms, curr_bpm, base_bpm, scroll_speed, curr_fp);
      }
    rlPopMatrix();
    rlEnableBackfaceCulling();
    rlEnableDepthTest();
  EndMode3D();

  BeginMode3D(camera);
    rlDisableBackfaceCulling();
    rlPushMatrix();
      rlScalef(1.7896f, 1.0f, 1.0f);
      // Arctaps
      for (int i = arctap_list->size - 1; i >= 0; i--)
      {
        ArcTapFP *arctap_fp = (ArcTapFP *)list_get(arctap_list, i);
        ArcTap *arctap = arctap_fp->arctap;
        ChartTimingGroup *tg = (ChartTimingGroup *)list_get(timing_groups, arctap->timing_group);
        if (hidegroup_actives[tg->value]) continue;
        if (tg->props.no_input && arctap->timing - current_ms < 0) continue;
        double curr_fp = curr_fps[arctap->timing_group];
        double z_pos    = floor_position_to_z(arctap->fp - curr_fp, base_bpm, scroll_speed);
        draw_arctap(&notes_service->arctap, arctap, z_pos);
      }
    rlPopMatrix();
    rlEnableBackfaceCulling();
  EndMode3D();
  rlSetClipPlanes(0.01f, 100.0f);
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
