#ifndef ARC_FORMULA_H
#define ARC_FORMULA_H
#include "data/gameplay_events/arc.h"
#include "data/gameplay_events/timing_event.h"
#include "data/chart_timing_groups/chart_timing_group.h"

float lane_to_world_x(float lane);
double z_to_floor_position(double z, float base_bpm, float scroll_speed);
double floor_position_to_z(double fp, float base_bpm, float scroll_speed);
void batch_fp_to_z(double *fp_list, double *out, int count, float base_bpm, float scroll_speed);
void recalculate_floor_position(ChartTimingGroup *timing_group);
TimingEvent *get_event_at(List *timing_events, int timing);
double get_floor_position(List *timing_events, int timing);
double get_fp_from_current(List *timing_events, int timing, int current_timing);
float arc_x_to_world(float x);
float arc_y_to_world(float y);
float calculate_arc_segment_length(int duration, float arc_resolution);
float _x(float start, float end, float t, ArcType type);
float _y(float start, float end, float t, ArcType type);
float arc_world_x_at(int timing, struct Arc *arc);
float arc_world_y_at(int timing, struct Arc *arc);

#endif // ARC_FORMULA_H
