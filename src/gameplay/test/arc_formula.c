#include <math.h>
#include "../../acutest.h"
#include "../../constants.h"
#include "../../data/custom_types/dynamic_list.c"
#include "../../data/gameplay_events/timing_event.c"
#include "../../data/gameplay_events/gameplay_events.c"
#include "../arc_formula.c"

static const float TEST_ARC_LENGTH  = 1000.0f / 14.0f;

static Arc make_arc(int start_timing, int end_timing, float x1, float y1, float x2, float y2, ArcType type)
{
  Arc arc = {0};
  arc.start_timing = start_timing;
  arc.end_timing   = end_timing;
  arc.x1 = x1; arc.y1 = y1;
  arc.x2 = x2; arc.y2 = y2;
  arc.type = type;
  return arc;
}

void test_z_to_fp(void){
  float base_bpm = 150.0f;
  float scroll_speed = 5.0f;
  float target_z = -100.0f;

  // --- Normal case ---
  double result = z_to_floor_position(target_z, base_bpm, scroll_speed);
  TEST_CHECK(fabs(result - 96000) < 1e-6);
  TEST_MSG("normal: expected 96000, got %f", result);

  // --- Below minimum scroll speed -> clamped to MINIMUM_SCROLL_SPEED ---
  float scroll_speed_below_min = MINIMUM_SCROLL_SPEED - 1.0f;
  double expected_below_min = z_to_floor_position(target_z, base_bpm, MINIMUM_SCROLL_SPEED);
  double produced_below_min = z_to_floor_position(target_z, base_bpm, scroll_speed_below_min);
  TEST_CHECK(fabs(expected_below_min - produced_below_min) < 1e-6);
  TEST_MSG("below min: expected %f (clamped), got %f", expected_below_min, produced_below_min);

  // --- Above maximum scroll speed -> clamped to MAXIMUM_SCROLL_SPEED ---
  float scroll_speed_above_max = MAXIMUM_SCROLL_SPEED + 1.0f;
  double expected_above_max = z_to_floor_position(target_z, base_bpm, MAXIMUM_SCROLL_SPEED);
  double produced_above_max = z_to_floor_position(target_z, base_bpm, scroll_speed_above_max);
  TEST_CHECK(fabs(expected_above_max - produced_above_max) < 1e-6);
  TEST_MSG("above max: expected %f (clamped), got %f", expected_above_max, produced_above_max);
}

void test_fp_to_z(void){
  float base_bpm = 150.0f;
  float scroll_speed = 5.0f;
  double target_fp = 96000.0;

  // --- Normal case ---
  float result = floor_position_to_z(target_fp, base_bpm, scroll_speed);
  TEST_CHECK(fabsf(result - (-100.0f)) < 1e-6);
  TEST_MSG("normal: expected -100, got %f", result);

  // --- Below minimum scroll speed -> clamped to MINIMUM_SCROLL_SPEED ---
  float scroll_speed_below_min = MINIMUM_SCROLL_SPEED - 1.0f;
  float expected_below_min = floor_position_to_z(target_fp, base_bpm, MINIMUM_SCROLL_SPEED);
  float produced_below_min = floor_position_to_z(target_fp, base_bpm, scroll_speed_below_min);
  TEST_CHECK(fabsf(expected_below_min - produced_below_min) < 1e-6);
  TEST_MSG("below min: expected %f (clamped), got %f", expected_below_min, produced_below_min);

  // --- Above maximum scroll speed -> clamped to MAXIMUM_SCROLL_SPEED ---
  float scroll_speed_above_max = MAXIMUM_SCROLL_SPEED + 1.0f;
  float expected_above_max = floor_position_to_z(target_fp, base_bpm, MAXIMUM_SCROLL_SPEED);
  float produced_above_max = floor_position_to_z(target_fp, base_bpm, scroll_speed_above_max);
  TEST_CHECK(fabsf(expected_above_max - produced_above_max) < 1e-6);
  TEST_MSG("above max: expected %f (clamped), got %f", expected_above_max, produced_above_max);
}

void test_zfp_round_trip(void){
  float base_bpm = 150.0f;
  float scroll_speed = 5.0f;

  // z = -100 -> fp->z should return -100
  float test_z = -100.0f;
  double test_fp = z_to_floor_position(test_z, base_bpm, scroll_speed);
  TEST_CHECK(fabs(test_fp - 96000) < 1e-6);
  TEST_MSG("z=-100: expected fp 96000, got %f", test_fp);
  float z_back = floor_position_to_z(test_fp, base_bpm, scroll_speed);
  TEST_CHECK(fabsf(z_back - test_z) < 1e-6);
  TEST_MSG("round-trip z=-100: expected %f, got %f", test_z, z_back);

  // z = -250 -> fp->z should return -250
  float test_z_1 = -250.0f;
  double test_fp_1 = z_to_floor_position(test_z_1, base_bpm, scroll_speed);
  TEST_CHECK(fabs(test_fp_1 - 240000) < 1e-6);
  TEST_MSG("z=-250: expected fp 240000, got %f", test_fp_1);
  float z_back_1 = floor_position_to_z(test_fp_1, base_bpm, scroll_speed);
  TEST_CHECK(fabsf(z_back_1 - test_z_1) < 1e-6);
  TEST_MSG("round-trip z=-250: expected %f, got %f", test_z_1, z_back_1);

  // z = 150 (positive) -> fp->z should return 150
  float test_z_2 = 150.0f;
  double test_fp_2 = z_to_floor_position(test_z_2, base_bpm, scroll_speed);
  TEST_CHECK(fabs(test_fp_2 - (-144000)) < 1e-6);
  TEST_MSG("z=150: expected fp -144000, got %f", test_fp_2);
  float z_back_2 = floor_position_to_z(test_fp_2, base_bpm, scroll_speed);
  TEST_CHECK(fabsf(z_back_2 - test_z_2) < 1e-6);
  TEST_MSG("round-trip z=150: expected %f, got %f", test_z_2, z_back_2);
}

void test_calculate_arc_segment_length(void)
{
  int duration_500 = 500;
  int duration_1000 = 1000;

  // --- Normal resolution (2.0): length = ARC_SEGMENT_LENGTH / res ---
  float arc_resolution = 2.0f;
  float result_500 = calculate_arc_segment_length(duration_500, arc_resolution);
  float result_1000 = calculate_arc_segment_length(duration_1000, arc_resolution);
  TEST_CHECK(fabsf(result_500 - (TEST_ARC_LENGTH / arc_resolution)) < 1e-6);
  TEST_MSG("res=2 dur=500: expected %f, got %f", TEST_ARC_LENGTH / arc_resolution, result_500);
  TEST_CHECK(fabsf(result_1000 - ((TEST_ARC_LENGTH / arc_resolution)*2)) < 1e-6);
  TEST_MSG("res=2 dur=1000: expected %f (x2), got %f", (TEST_ARC_LENGTH / arc_resolution)*2, result_1000);

  // --- Zero resolution -> returns duration as-is ---
  float arc_resolution_0 = 0.0f;
  result_500 = calculate_arc_segment_length(duration_500, arc_resolution_0);
  result_1000 = calculate_arc_segment_length(duration_1000, arc_resolution_0);
  TEST_CHECK(fabsf(result_500 - duration_500) < 1e-6);
  TEST_MSG("res=0 dur=500: expected %d, got %f", duration_500, result_500);
  TEST_CHECK(fabsf(result_1000 - duration_1000) < 1e-6);
  TEST_MSG("res=0 dur=1000: expected %d, got %f", duration_1000, result_1000);

  // --- Minimum resolution (1.0): length = ARC_SEGMENT_LENGTH ---
  float arc_resolution_1 = 1.0f;
  result_500 = calculate_arc_segment_length(duration_500, arc_resolution_1);
  result_1000 = calculate_arc_segment_length(duration_1000, arc_resolution_1);
  TEST_CHECK(fabsf(result_500 - TEST_ARC_LENGTH) < 1e-6);
  TEST_MSG("res=1 dur=500: expected %f, got %f", TEST_ARC_LENGTH, result_500);
  TEST_CHECK(fabsf(result_1000 - (TEST_ARC_LENGTH * 2)) < 1e-6);
  TEST_MSG("res=1 dur=1000: expected %f (x2), got %f", TEST_ARC_LENGTH * 2, result_1000);

  // --- Above maximum resolution -> clamped to MAXIMUM_ARC_RES ---
  float arc_resolution_10 = 10.0f;
  float seg_10_500 = calculate_arc_segment_length(duration_500, arc_resolution_10);
  float seg_10_1000 = calculate_arc_segment_length(duration_1000, arc_resolution_10);
  float arc_resolution_11 = 11.0f;
  float seg_11_500 = calculate_arc_segment_length(duration_500, arc_resolution_11);
  float seg_11_1000 = calculate_arc_segment_length(duration_1000, arc_resolution_11);
  TEST_CHECK(fabsf(seg_10_500 - seg_11_500) < 1e-6);
  TEST_MSG("clamp dur=500: res=10 and res=11 should match (10=%f, 11=%f)", seg_10_500, seg_11_500);
  TEST_CHECK(fabsf(seg_10_1000 - seg_11_1000) < 1e-6);
  TEST_MSG("clamp dur=1000: res=10 and res=11 should match (10=%f, 11=%f)", seg_10_1000, seg_11_1000);
}

void test_simd_fp_to_z(void)
{
  // --- Compare fp->z results from SIMD and normal looping
  float base_bpm = 150.0f;
  float scroll_speed = 5.0f;
  double fp_list[6] = {0.0, 48000.0, 96000.0, 144000.0, 192000.0, 240000.0};
  float *simd_z_out = (float *)malloc(6 * sizeof(float));
  batch_fp_to_z(fp_list, simd_z_out, 6, base_bpm, scroll_speed);

  double fp_tracker = 0.0;
  float normal_z_out[6];
  for (int i = 0; i < 6; i++)
  {
    normal_z_out[i] = floor_position_to_z(fp_tracker, base_bpm, scroll_speed);
    fp_tracker += 48000.0;
  }

  // TODO: check simd results for correctness

  // check simd and normal results' same-ness
  for (int i = 0; i < 6; i++)
  {
    TEST_CHECK(fabsf(simd_z_out[i] - normal_z_out[i]) < 1e-6);
    TEST_MSG("simd result at index %d: %f; normal result at %d: %f", i, simd_z_out[i], i, normal_z_out[i]);
  }
  free(simd_z_out);
}

void test_get_world_at(void)
{
  // arc(0,1500,-0.50,0.00,b,0.00,0.00,0,none,true)[arctap(1500)];
  Arc arc_1 = make_arc(0, 1500, -0.50f, 0.00f, 0.00f, 0.00f, B);
  ArcTap arctap_1 = {.arc = &arc_1, .timing = 1500};
  float expected_x_1 = arc_x_to_world(0.0f);
  float expected_y_1 = arc_y_to_world(0.0f);
  float produced_x_1 = arc_world_x_at(arctap_1.timing, arctap_1.arc);
  float produced_y_1 = arc_world_y_at(arctap_1.timing, arctap_1.arc);
  TEST_CHECK(fabsf(expected_x_1 - produced_x_1) < 1e-6);
  TEST_MSG("Arc 1 expected x: %f; produced x: %f", expected_x_1, produced_x_1);
  TEST_CHECK(fabsf(expected_y_1 - produced_y_1) < 1e-6);
  TEST_MSG("Arc 1 expected y: %f; produced y: %f", expected_y_1, produced_y_1);

  // arc(0,1688,-0.50,0.50,b,0.00,0.00,0,none,true)[arctap(1688)];
  Arc arc_2 = make_arc(0, 1688, -0.50f, 0.00f, 0.50f, 0.00f, B);
  ArcTap arctap_2 = {.arc = &arc_2, .timing = 1688};
  float expected_x_2 = arc_x_to_world(0.5f);
  float expected_y_2 = arc_y_to_world(0.0f);
  float produced_x_2 = arc_world_x_at(arctap_2.timing, arctap_2.arc);
  float produced_y_2 = arc_world_y_at(arctap_2.timing, arctap_2.arc);
  TEST_CHECK(fabsf(expected_x_2 - produced_x_2) < 1e-6);
  TEST_MSG("Arc 2 expected x: %f; produced x: %f", expected_x_2, produced_x_2);
  TEST_CHECK(fabsf(expected_y_2 - produced_y_2) < 1e-6);
  TEST_MSG("Arc 2 expected y: %f; produced y: %f", expected_y_2, produced_y_2);

  // arc(0,1875,-0.50,1.00,b,0.00,0.00,0,none,true)[arctap(1875)];
  Arc arc_3 = make_arc(0, 1875, -0.50f, 0.00f, 1.00f, 0.00f, B);
  ArcTap arctap_3 = {.arc = &arc_3, .timing = 1875};
  float expected_x_3 = arc_x_to_world(1.0f);
  float expected_y_3 = arc_y_to_world(0.0f);
  float produced_x_3 = arc_world_x_at(arctap_3.timing, arctap_3.arc);
  float produced_y_3 = arc_world_y_at(arctap_3.timing, arctap_3.arc);
  TEST_CHECK(fabsf(expected_x_3 - produced_x_3) < 1e-6);
  TEST_MSG("Arc 3 expected x: %f; produced x: %f", expected_x_3, produced_x_3);
  TEST_CHECK(fabsf(expected_y_3 - produced_y_3) < 1e-6);
  TEST_MSG("Arc 3 expected y: %f; produced y: %f", expected_y_3, produced_y_3);

  // arc(23625,23625,1.00,1.00,soso,1.00,0.00, none, true)
  Arc arc_zd = make_arc(23625, 23625, 1.00f, 1.00f, 1.00f, 0.00f, SOSO);
  // arctap at the arc's timing → world(x1, y1)
  TEST_CHECK(fabsf(arc_world_x_at(23625, &arc_zd) - arc_x_to_world(1.00f)) < 1e-6);
  TEST_CHECK(fabsf(arc_world_y_at(23625, &arc_zd) - arc_y_to_world(1.00f)) < 1e-6);
  // timing after the arc → world(x2, y2)
  TEST_CHECK(fabsf(arc_world_x_at(23626, &arc_zd) - arc_x_to_world(1.00f)) < 1e-6);
  TEST_CHECK(fabsf(arc_world_y_at(23626, &arc_zd) - arc_y_to_world(0.00f)) < 1e-6);
}

TEST_LIST = {
  {"z_to_fp", test_z_to_fp},
  {"fp_to_z", test_fp_to_z},
  {"zfp_round_trip", test_zfp_round_trip},
  {"calculate_arc_segment_length", test_calculate_arc_segment_length},
  {"simd_fp_to_z", test_simd_fp_to_z},
  {"get_world_at", test_get_world_at},
  { NULL, NULL }
};
