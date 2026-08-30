#include <math.h>
#include "../../acutest.h"
#include "../../constants.h"
#include "../../data/custom_types/custom_types.c"
#include "../../data/gameplay_events/timing_event.c"
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
  float result = z_to_floor_position(target_z, base_bpm, scroll_speed);
  TEST_CHECK(fabsf(result - 96000) < 1e-6);
  TEST_MSG("normal: expected 96000, got %f", result);

  // --- Below minimum scroll speed -> clamped to MINIMUM_SCROLL_SPEED ---
  float scroll_speed_below_min = MINIMUM_SCROLL_SPEED - 1.0f;
  float expected_below_min = z_to_floor_position(target_z, base_bpm, MINIMUM_SCROLL_SPEED);
  float produced_below_min = z_to_floor_position(target_z, base_bpm, scroll_speed_below_min);
  TEST_CHECK(fabsf(expected_below_min - produced_below_min) < 1e-6);
  TEST_MSG("below min: expected %f (clamped), got %f", expected_below_min, produced_below_min);

  // --- Above maximum scroll speed -> clamped to MAXIMUM_SCROLL_SPEED ---
  float scroll_speed_above_max = MAXIMUM_SCROLL_SPEED + 1.0f;
  float expected_above_max = z_to_floor_position(target_z, base_bpm, MAXIMUM_SCROLL_SPEED);
  float produced_above_max = z_to_floor_position(target_z, base_bpm, scroll_speed_above_max);
  TEST_CHECK(fabsf(expected_above_max - produced_above_max) < 1e-6);
  TEST_MSG("above max: expected %f (clamped), got %f", expected_above_max, produced_above_max);
}

void test_fp_to_z(void){
  float base_bpm = 150.0f;
  float scroll_speed = 5.0f;
  float target_fp = 96000.0f;

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
  float test_fp = z_to_floor_position(test_z, base_bpm, scroll_speed);
  TEST_CHECK(fabsf(test_fp - 96000) < 1e-6);
  TEST_MSG("z=-100: expected fp 96000, got %f", test_fp);
  float z_back = floor_position_to_z(test_fp, base_bpm, scroll_speed);
  TEST_CHECK(fabsf(z_back - test_z) < 1e-6);
  TEST_MSG("round-trip z=-100: expected %f, got %f", test_z, z_back);

  // z = -250 -> fp->z should return -250
  float test_z_1 = -250.0f;
  float test_fp_1 = z_to_floor_position(test_z_1, base_bpm, scroll_speed);
  TEST_CHECK(fabsf(test_fp_1 - 240000) < 1e-6);
  TEST_MSG("z=-250: expected fp 240000, got %f", test_fp_1);
  float z_back_1 = floor_position_to_z(test_fp_1, base_bpm, scroll_speed);
  TEST_CHECK(fabsf(z_back_1 - test_z_1) < 1e-6);
  TEST_MSG("round-trip z=-250: expected %f, got %f", test_z_1, z_back_1);

  // z = 150 (positive) -> fp->z should return 150
  float test_z_2 = 150.0f;
  float test_fp_2 = z_to_floor_position(test_z_2, base_bpm, scroll_speed);
  TEST_CHECK(fabsf(test_fp_2 - (-144000)) < 1e-6);
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

void test_arc_world_x_at(void)
{
  // --- Zero-duration guard: start == end -> returns fallback, ignores x1/x2/type ---
  Arc arc_zero = make_arc(500, 500, 0.2f, 0.3f, 0.8f, 0.9f, S);
  float fallback = 0.5f;
  float result = arc_world_x_at(500, &arc_zero, fallback);
  float expected = arc_x_to_world(fallback);
  TEST_CHECK(fabsf(result - expected) < 1e-6f);
  TEST_MSG("zero-duration: expected fallback %f, got %f", expected, result);
  // verify it did NOT use x1
  TEST_CHECK(fabsf(result - arc_x_to_world(arc_zero.x1)) > 1e-6f);
  TEST_MSG("zero-duration: should not use x1 (%f)", arc_x_to_world(arc_zero.x1));

  // --- At start: timing == start_timing -> p=0 -> arc_x_to_world(x1) ---
  Arc arc = make_arc(100, 500, 0.2f, 0.3f, 0.8f, 0.9f, S);
  result = arc_world_x_at(100, &arc, 0.0f);
  expected = arc_x_to_world(0.2f);
  TEST_CHECK(fabsf(result - expected) < 1e-6f);
  TEST_MSG("at start: expected %f, got %f", expected, result);

  // --- At end: timing == end_timing -> p=1 -> arc_x_to_world(x2) ---
  result = arc_world_x_at(500, &arc, 0.0f);
  expected = arc_x_to_world(0.8f);
  TEST_CHECK(fabsf(result - expected) < 1e-6f);
  TEST_MSG("at end: expected %f, got %f", expected, result);

  // --- Before start: timing < start_timing -> Clamp to p=0 -> x1 ---
  result = arc_world_x_at(50, &arc, 0.0f);
  expected = arc_x_to_world(0.2f);
  TEST_CHECK(fabsf(result - expected) < 1e-6f);
  TEST_MSG("before start (clamp): expected %f, got %f", expected, result);

  // --- After end: timing > end_timing -> Clamp to p=1 -> x2 ---
  result = arc_world_x_at(999, &arc, 0.0f);
  expected = arc_x_to_world(0.8f);
  TEST_CHECK(fabsf(result - expected) < 1e-6f);
  TEST_MSG("after end (clamp): expected %f, got %f", expected, result);

  // --- Midpoint with linear type S: p=0.5 -> (x1+x2)/2 ---
  result = arc_world_x_at(300, &arc, 0.0f);  // (100+500)/2 = 300
  expected = arc_x_to_world((0.2f + 0.8f) / 2.0f);
  TEST_CHECK(fabsf(result - expected) < 1e-6f);
  TEST_MSG("midpoint S: expected %f, got %f", expected, result);
}

void test_arc_world_y_at(void)
{
  // --- Zero-duration guard: start == end -> returns fallback ---
  Arc arc_zero = make_arc(500, 500, 0.2f, 0.3f, 0.8f, 0.9f, S);
  float fallback = 0.5f;
  float result = arc_world_y_at(500, &arc_zero, fallback);
  float expected = arc_y_to_world(fallback);
  TEST_CHECK(fabsf(result - expected) < 1e-6f);
  TEST_MSG("zero-duration: expected fallback %f, got %f", expected, result);
  TEST_CHECK(fabsf(result - arc_y_to_world(arc_zero.y1)) > 1e-6f);
  TEST_MSG("zero-duration: should not use y1 (%f)", arc_y_to_world(arc_zero.y1));

  Arc arc = make_arc(100, 500, 0.2f, 0.3f, 0.8f, 0.9f, S);

  // --- At start: p=0 -> y1 ---
  result = arc_world_y_at(100, &arc, 0.0f);
  expected = arc_y_to_world(0.3f);
  TEST_CHECK(fabsf(result - expected) < 1e-6f);
  TEST_MSG("at start: expected %f, got %f", expected, result);

  // --- At end: p=1 -> y2 ---
  result = arc_world_y_at(500, &arc, 0.0f);
  expected = arc_y_to_world(0.9f);
  TEST_CHECK(fabsf(result - expected) < 1e-6f);
  TEST_MSG("at end: expected %f, got %f", expected, result);

  // --- Before start: Clamp -> p=0 -> y1 ---
  result = arc_world_y_at(50, &arc, 0.0f);
  TEST_CHECK(fabsf(result - arc_y_to_world(0.3f)) < 1e-6f);
  TEST_MSG("before start (clamp): expected %f, got %f", arc_y_to_world(0.3f), result);

  // --- After end: Clamp -> p=1 -> y2 ---
  result = arc_world_y_at(999, &arc, 0.0f);
  TEST_CHECK(fabsf(result - arc_y_to_world(0.9f)) < 1e-6f);
  TEST_MSG("after end (clamp): expected %f, got %f", arc_y_to_world(0.9f), result);

  // --- Midpoint with linear type S: p=0.5 -> (y1+y2)/2 ---
  result = arc_world_y_at(300, &arc, 0.0f);
  expected = arc_y_to_world((0.3f + 0.9f) / 2.0f);
  TEST_CHECK(fabsf(result - expected) < 1e-6f);
  TEST_MSG("midpoint S: expected %f, got %f", expected, result);
}

void test_arc_world_at_all_types(void)
{
  // --- Validates all easing function returns start at t=0 and end at t=1 ---
  ArcType all_types[] = { B, S, SI, SO, SISI, SOSO, SISO, SOSI };
  int num_types = (int)(sizeof(all_types) / sizeof(all_types[0]));

  for (int i = 0; i < num_types; i++)
  {
    Arc arc = make_arc(100, 500, 0.2f, 0.3f, 0.8f, 0.9f, all_types[i]);

    // p=0 (at start_timing) -> should be x1, y1
    float x_at_start = arc_world_x_at(100, &arc, 0.0f);
    float y_at_start = arc_world_y_at(100, &arc, 0.0f);
    TEST_CHECK(fabsf(x_at_start - arc_x_to_world(0.2f)) < 1e-6f);
    TEST_CHECK(fabsf(y_at_start - arc_y_to_world(0.3f)) < 1e-6f);
    TEST_MSG("type=%d at start: x expected %f got %f, y expected %f got %f",
             all_types[i], arc_x_to_world(0.2f), x_at_start, arc_y_to_world(0.3f), y_at_start);

    // p=1 (at end_timing) -> should be x2, y2
    float x_at_end = arc_world_x_at(500, &arc, 0.0f);
    float y_at_end = arc_world_y_at(500, &arc, 0.0f);
    TEST_CHECK(fabsf(x_at_end - arc_x_to_world(0.8f)) < 1e-6f);
    TEST_CHECK(fabsf(y_at_end - arc_y_to_world(0.9f)) < 1e-6f);
    TEST_MSG("type=%d at end: x expected %f got %f, y expected %f got %f",
             all_types[i], arc_x_to_world(0.8f), x_at_end, arc_y_to_world(0.9f), y_at_end);
  }
}

void test_simd_fp_to_z(void)
{
  // --- Compare fp->z results from SIMD and normal looping
  float base_bpm = 150.0f;
  float scroll_speed = 5.0f;
  float fp_list[6] = {0.0f, 48000.0f, 96000.0f, 144000.0f, 192000.0f, 240000.0f};
  float *simd_z_out = (float *)malloc(6 * sizeof(float));
  batch_fp_to_z((float *)&fp_list, simd_z_out, 6, base_bpm, scroll_speed);

  float fp_tracker = 0.0f;
  float normal_z_out[6];
  for (int i = 0; i < 6; i++)
  {
    normal_z_out[i] = floor_position_to_z(fp_tracker, base_bpm, scroll_speed);
    fp_tracker += 48000.0f;
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

TEST_LIST = {
  {"z_to_fp", test_z_to_fp},
  {"fp_to_z", test_fp_to_z},
  {"zfp_round_trip", test_zfp_round_trip},
  {"calculate_arc_segment_length", test_calculate_arc_segment_length},
  {"arc_world_x_at", test_arc_world_x_at},
  {"arc_world_y_at", test_arc_world_y_at},
  {"arc_world_at_all_types", test_arc_world_at_all_types},
  {"simd_fp_to_z", test_simd_fp_to_z},
  { NULL, NULL }
};
