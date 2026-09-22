#include "../../../acutest.h"
#include <math.h>
#include "../easings.c"

#define EPS 1e-6f

static const EasingType ALL[] = {
  E_LINEAR, E_STEP_START, E_STEP_END,
  E_OUT_SINE, E_INOUT_SINE, E_IN_SINE,
  E_OUT_QUART, E_INOUT_QUART, E_IN_QUART,
  E_OUT_QUAD, E_INOUT_QUAD, E_IN_QUAD,
  E_OUT_QUINT, E_INOUT_QUINT, E_IN_QUINT,
  E_OUT_EXPO, E_INOUT_EXPO, E_IN_EXPO,
  E_OUT_ELASTIC, E_INOUT_ELASTIC, E_IN_ELASTIC,
  E_OUT_CUBIC, E_INOUT_CUBIC, E_IN_CUBIC,
  E_OUT_CIRC, E_INOUT_CIRC, E_IN_CIRC,
  E_OUT_BOUNCE, E_INOUT_BOUNCE, E_IN_BOUNCE,
  E_OUT_BACK, E_INOUT_BACK, E_IN_BACK,
};
static const int N_EASINGS = (int)(sizeof(ALL) / sizeof(ALL[0]));

/* Easings that must stay inside [0, 1] and rise steadily. */
static const EasingType MONOTONIC[] = {
  E_LINEAR,
  E_OUT_SINE, E_INOUT_SINE, E_IN_SINE,
  E_OUT_QUAD, E_INOUT_QUAD, E_IN_QUAD,
  E_OUT_CUBIC, E_INOUT_CUBIC, E_IN_CUBIC,
  E_OUT_QUART, E_INOUT_QUART, E_IN_QUART,
  E_OUT_QUINT, E_INOUT_QUINT, E_IN_QUINT,
  E_OUT_EXPO, E_INOUT_EXPO, E_IN_EXPO,
  E_OUT_CIRC, E_INOUT_CIRC, E_IN_CIRC,
};
static const int N_MONOTONIC = (int)(sizeof(MONOTONIC) / sizeof(MONOTONIC[0]));

static void expect_unit(EasingType e, float t, float expected)
{
  float got = easing_get_unit(e, t);
  TEST_CHECK_(fabsf(got - expected) < EPS,
              "%s at t=%.3f: expected %.5f, got %.5f",
              "easing", t, expected, got);
}

/* 1. Every easing except the step types must map 0 -> 0 and 1 -> 1. */
void test_unit_endpoints(void)
{
  for (int i = 0; i < N_EASINGS; i++) {
    EasingType e = ALL[i];
    if (e == E_STEP_START || e == E_STEP_END) continue;
    expect_unit(e, 0.0f, 0.0f);
    expect_unit(e, 1.0f, 1.0f);
  }
}

/* 2. Step easings jump between 0 and 1. */
void test_unit_steps(void)
{
  TEST_CHECK(easing_get_unit(E_STEP_START, 0.0f) == 0.0f);
  TEST_CHECK(easing_get_unit(E_STEP_START, 0.001f) == 1.0f);
  TEST_CHECK(easing_get_unit(E_STEP_END, 0.999f) == 0.0f);
  TEST_CHECK(easing_get_unit(E_STEP_END, 1.0f) == 0.0f);
  TEST_CHECK(easing_get_unit(E_STEP_END, 1.001f) == 1.0f);
}

/* 3. Quarter-point values. These pin each formula to easings.net. */
void test_unit_known_values(void)
{
  expect_unit(E_LINEAR,      0.25f, 0.25f);
  expect_unit(E_LINEAR,      0.50f, 0.5f);
  expect_unit(E_IN_QUAD,     0.5f , 0.25f);
  expect_unit(E_OUT_QUAD,    0.5f , 0.75f);
  expect_unit(E_INOUT_QUAD,  0.25f, 0.125f);
  expect_unit(E_IN_CUBIC,    0.5f , 0.125f);
  expect_unit(E_OUT_CUBIC,   0.5f , 0.875f);
  expect_unit(E_INOUT_CUBIC, 0.25f, 0.0625f);
  expect_unit(E_IN_QUART,    0.5f , 0.0625f);
  expect_unit(E_OUT_QUART,   0.5f , 0.9375f);
  expect_unit(E_IN_QUINT,    0.5f , 0.03125f);
  expect_unit(E_OUT_QUINT,   0.5f , 0.96875f);
  expect_unit(E_INOUT_QUINT, 0.25f, 0.015625f);
  expect_unit(E_IN_SINE,     0.5f , 1.0f - cosf(0.25f * (float)M_PI));
  expect_unit(E_OUT_SINE,    0.5f , sinf(0.25f * (float)M_PI));
  expect_unit(E_INOUT_SINE,  0.5f , 0.5f);
  expect_unit(E_IN_EXPO,     0.5f , powf(2.0f, -5.0f));
  expect_unit(E_OUT_EXPO,    0.5f , 1.0f - powf(2.0f, -5.0f));
  expect_unit(E_INOUT_EXPO,  0.25f, powf(2.0f, -5.0f) * 0.5f);
  expect_unit(E_IN_CIRC,     0.5f , 1.0f - sqrtf(0.75f));
  expect_unit(E_OUT_CIRC,    0.5f , sqrtf(0.75f));
  expect_unit(E_IN_BACK,     0.5f , 2.70158f * 0.125f - 1.70158f * 0.25f);
  expect_unit(E_IN_BOUNCE,   0.25f, 1.0f - ease_out_bounce(0.75f));
}

/* out bounce is a static helper. Reach it through E_IN_BOUNCE:
   in_bounce(t) = 1 - out_bounce(1 - t). So out_bounce(0.25) can be
   checked via a public path. Test the piecewise branches directly. */
static float bounce_ref(float t)
{
  const float n1 = 7.5625f, d1 = 2.75f;
  if (t < 1.0f / d1) return n1 * t * t;
  if (t < 2.0f / d1) { t -= 1.5f / d1; return n1 * t * t + 0.75f; }
  if (t < 2.5f / d1) { t -= 2.25f / d1; return n1 * t * t + 0.9375f; }
  t -= 2.625f / d1;  return n1 * t * t + 0.984375f;
}

void test_unit_bounce_branches(void)
{
  /* One sample from each of the four piecewise segments. */
  float samples[] = { 0.1f, 0.45f, 0.75f, 0.95f };
  for (int i = 0; i < 4; i++) {
    float t = samples[i];
    expect_unit(E_OUT_BOUNCE, t, bounce_ref(t));
    expect_unit(E_IN_BOUNCE, t, 1.0f - bounce_ref(1.0f - t));
  }
  /* Bounce hits its first landing exactly. */
  expect_unit(E_OUT_BOUNCE, 1.0f / 2.75f, 1.0f);
}

/* 4. Monotone easings stay in [0, 1] and never decrease. */
void test_unit_monotonic_in_range(void)
{
  for (int i = 0; i < N_MONOTONIC; i++) {
    EasingType e = MONOTONIC[i];
    float prev = easing_get_unit(e, 0.0f);
    TEST_CHECK(prev >= -EPS);
    for (int k = 1; k <= 100; k++) {
      float t = k / 100.0f;
      float v = easing_get_unit(e, t);
      if (!TEST_CHECK_(v >= prev - EPS,
                       "monotonic easing %d not rising at t=%.2f", (int)e, t))
        break;
      if (!TEST_CHECK_(v <= 1.0f + EPS && v >= -EPS,
                       "monotonic easing %d out of range at t=%.2f (v=%.4f)",
                       (int)e, t, v))
        break;
      prev = v;
    }
  }
}

/* 5. Overshoot easings must exceed the [0, 1] range somewhere.
   Back and elastic go below 0 near the start and above 1 near the end
   (in variants), or vice versa (out variants). */
void test_unit_overshoot(void)
{
  TEST_CHECK(easing_get_unit(E_IN_BACK, 0.4f)  < 0.0f);
  TEST_CHECK(easing_get_unit(E_OUT_BACK, 0.6f) > 1.0f);
  TEST_CHECK(easing_get_unit(E_IN_ELASTIC, 0.2f) < 0.0f);
  TEST_CHECK(easing_get_unit(E_OUT_ELASTIC, 0.8f) > 1.0f);
}

/* 6. inout is the mirror of itself: f(t) + f(1 - t) = 1 for all inout
   easings. This catches asymmetric formulas. */
void test_unit_inout_symmetry(void)
{
  EasingType inout[] = {
    E_INOUT_SINE, E_INOUT_QUAD, E_INOUT_CUBIC, E_INOUT_QUART,
    E_INOUT_QUINT, E_INOUT_EXPO, E_INOUT_ELASTIC, E_INOUT_CIRC,
    E_INOUT_BOUNCE, E_INOUT_BACK,
  };
  for (int i = 0; i < (int)(sizeof(inout) / sizeof(inout[0])); i++) {
    for (int k = 0; k <= 20; k++) {
      float t = k / 20.0f;
      float a = easing_get_unit(inout[i], t);
      float b = easing_get_unit(inout[i], 1.0f - t);
      if (!TEST_CHECK_(fabsf(a + b - 1.0f) < EPS,
                       "inout %d not symmetric at t=%.2f (%.4f + %.4f)",
                       (int)inout[i], t, a, b))
        break;
    }
  }
}

/* 7. in and out are inverses: out_e(t) == 1 - in_e(1 - t).
   This holds for the whole polynomial, sine, expo, circ, bounce, and
   back families. */
void test_unit_in_out_inverse(void)
{
  struct { EasingType in_e, out_e; } pairs[] = {
    { E_IN_SINE,    E_OUT_SINE },
    { E_IN_QUAD,    E_OUT_QUAD },
    { E_IN_CUBIC,   E_OUT_CUBIC },
    { E_IN_QUART,   E_OUT_QUART },
    { E_IN_QUINT,   E_OUT_QUINT },
    { E_IN_EXPO,    E_OUT_EXPO },
    { E_IN_CIRC,    E_OUT_CIRC },
    { E_IN_BOUNCE,  E_OUT_BOUNCE },
    { E_IN_BACK,    E_OUT_BACK },
  };
  for (int i = 0; i < (int)(sizeof(pairs) / sizeof(pairs[0])); i++) {
    for (int k = 1; k < 20; k++) {
      float t = k / 20.0f;
      float out = easing_get_unit(pairs[i].out_e, t);
      float inv = 1.0f - easing_get_unit(pairs[i].in_e, 1.0f - t);
      if (!TEST_CHECK_(fabsf(out - inv) < EPS,
                       "in/out pair (%d, %d) mismatch at t=%.2f (%.4f vs %.4f)",
                       (int)pairs[i].in_e, (int)pairs[i].out_e, t, out, inv))
        break;
    }
  }
}

/* 8. inout is built from in and out:
   inout(t) = 0.5 * in(2t)      for t < 0.5
   inout(t) = 0.5 + 0.5 * out(2t - 1)  for t >= 0.5 */
void test_unit_inout_composition(void)
{
  struct { EasingType in_e, out_e, inout_e; } trios[] = {
    { E_IN_SINE, E_OUT_SINE, E_INOUT_SINE },
    { E_IN_QUAD, E_OUT_QUAD, E_INOUT_QUAD },
    { E_IN_CUBIC, E_OUT_CUBIC, E_INOUT_CUBIC },
    { E_IN_QUART, E_OUT_QUART, E_INOUT_QUART },
    { E_IN_QUINT, E_OUT_QUINT, E_INOUT_QUINT },
  };
  for (int i = 0; i < (int)(sizeof(trios) / sizeof(trios[0])); i++) {
    for (int k = 1; k < 20; k++) {
      float t = k / 20.0f;
      float expected, got = easing_get_unit(trios[i].inout_e, t);
      if (t < 0.5f)
        expected = 0.5f * easing_get_unit(trios[i].in_e, 2.0f * t);
      else
        expected = 0.5f + 0.5f * easing_get_unit(trios[i].out_e, 2.0f * t - 1.0f);
      if (!TEST_CHECK_(fabsf(got - expected) < EPS,
                       "inout %d does not match in/out halves at t=%.2f",
                       (int)trios[i].inout_e, t))
        break;
    }
  }
}

/* 9. interpolate: from and to are respected, and the range maps. */
void test_interpolate_values(void)
{
  float r;

  r = easing_interpolate(E_LINEAR, 500, 0, 1000, 10.0f, 30.0f);
  TEST_CHECK(fabsf(r - 20.0f) < EPS);

  /* Reverse direction. */
  r = easing_interpolate(E_LINEAR, 250, 0, 1000, 30.0f, 10.0f);
  TEST_CHECK(fabsf(r - 25.0f) < EPS);

  /* Non-zero start timing. */
  r = easing_interpolate(E_LINEAR, 300, 200, 400, 0.0f, 2.0f);
  TEST_CHECK(fabsf(r - 1.0f) < EPS);

  /* Negative timing range. */
  r = easing_interpolate(E_LINEAR, -500, -1000, 0, 0.0f, 1.0f);
  TEST_CHECK(fabsf(r - 0.5f) < EPS);

  /* start == end returns from. */
  r = easing_interpolate(E_LINEAR, 0, 500, 500, 1.0f, 9.0f);
  TEST_CHECK(r == 1.0f);

  /* from == to returns from, even when the unit is undefined. */
  r = easing_interpolate(E_IN_BACK, 500, 0, 1000, 5.0f, 5.0f);
  TEST_CHECK(r == 5.0f);
}

/* 10. interpolate at the boundaries returns from and to for every
   easing, including the overshoot types. */
void test_interpolate_endpoints_all_easings(void)
{
  for (int i = 0; i < N_EASINGS; i++) {
    EasingType e = ALL[i];
    float lo = easing_interpolate(e, 0, 0, 1000, 10.0f, 90.0f);
    float hi = easing_interpolate(e, 1000, 0, 1000, 10.0f, 90.0f);
    if (e == E_STEP_END) {
      TEST_CHECK_(lo == 10.0f, "step end at t=0 must return from");
      TEST_CHECK_(hi == 10.0f, "step end at t=1 must return from");
      continue;
    }
    TEST_CHECK_(fabsf(lo - 10.0f) < EPS, "easing %d: t=0 must give from", (int)e);
    TEST_CHECK_(fabsf(hi - 90.0f) < EPS, "easing %d: t=1 must give to", (int)e);
  }
}

/* 11. Timing past the range extrapolates. The code does not clamp.
   Linear extrapolation must extend the line. Document the current
   behavior so a future clamp does not break silently. */
void test_interpolate_extrapolation(void)
{
  float r;

  r = easing_interpolate(E_LINEAR, -500, 0, 1000, 0.0f, 1.0f);
  TEST_CHECK(fabsf(r - (-0.5f)) < EPS);

  r = easing_interpolate(E_LINEAR, 1500, 0, 1000, 0.0f, 1.0f);
  TEST_CHECK(fabsf(r - 1.5f) < EPS);
}

TEST_LIST = {
  {"unit_endpoints",            test_unit_endpoints},
  {"unit_steps",                test_unit_steps},
  {"unit_known_values",         test_unit_known_values},
  {"unit_bounce_branches",      test_unit_bounce_branches},
  {"unit_monotonic_in_range",   test_unit_monotonic_in_range},
  {"unit_overshoot",            test_unit_overshoot},
  {"unit_inout_symmetry",       test_unit_inout_symmetry},
  {"unit_in_out_inverse",       test_unit_in_out_inverse},
  {"unit_inout_composition",    test_unit_inout_composition},
  {"interpolate_values",        test_interpolate_values},
  {"interpolate_endpoints",     test_interpolate_endpoints_all_easings},
  {"interpolate_extrapolation", test_interpolate_extrapolation},
  { NULL, NULL }
};
