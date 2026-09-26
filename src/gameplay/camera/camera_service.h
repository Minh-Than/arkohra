#ifndef CAMERA_SERVICE_H
#define CAMERA_SERVICE_H

#include "data/keyframe/value_channel.h"
#include "raylib.h"

Camera3D camera_init_playfield();
float get_aspect_ratio_adjustment();
Vector3 unity_quarternion_to_forward(Vector3 unityQuaternionDeg);
void recalibrate_camera(Camera3D *camera);
void camera_handle_main_window_resize(Camera3D *camera, float playfield_ratio);
Vector3 project_to_camera_view(Camera3D camera, float distance, float offsetX, float offsetY);
Camera3D get_enwidened_camera(Camera3D camera, ValueChannel *enwidencamera_channel, float current_ms);
// Vector2 get_frustum_size_at_distance(Camera3D camera, float distance);

#endif // CAMERA_SERVICE_H
