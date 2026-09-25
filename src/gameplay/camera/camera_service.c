#include <math.h>
#include <stdio.h>
#include "camera_service.h"
#include "raylib.h"
#include "raymath.h"
#include "../src/constants.h"

Camera3D camera_init_playfield()
{
  Camera3D camera = {
    .position   = (Vector3){ 0.0f, CAMERA_Y, CAMERA_Z },
    .target     = Vector3Zero(),                            // Camera looking at point
    .up         = (Vector3){ 0.0f, 1.0f, 0.0f },            // Camera up vector (rotation towards target)
    .fovy       = 50,                                       // Camera field-of-view Y
    .projection = CAMERA_PERSPECTIVE,                       // Perspective or isometric
  };
  recalibrate_camera(&camera);
  return camera;
}

float get_aspect_ratio_adjustment()
{
  float pixel_height        = (float)GetScreenHeight() / ((float)GetScreenWidth() / 16.0f);
  float aspect_adjustment   = fmaxf(0.0f, fminf(1.0f, (pixel_height - 9.0f) / 3.0f));
  return aspect_adjustment;
}

// Vector2 get_frustum_size_at_distance(Camera3D camera, float distance)
// {
//     float aspect = (float)GetScreenWidth() / (float)GetScreenHeight();
//     float height = 2.0f * distance * tanf(DEG2RAD * camera.fovy * 0.5f);
//     float width  = height * aspect;
// 
//     return (Vector2){ width, height };
// }

Vector3 unity_quarternion_to_forward(Vector3 unityQuaternionDeg)
{
  // Unity composes rotation as: Z first, then X, then Y
  // raylib's MatrixMultiply(A, B) applies B first — nest it to match that same order
  Matrix rot = MatrixMultiply(
    MatrixRotateY(unityQuaternionDeg.y * DEG2RAD),
    MatrixMultiply(
      MatrixRotateX(-unityQuaternionDeg.x * DEG2RAD),
      MatrixRotateZ(unityQuaternionDeg.z * DEG2RAD)
    )
  );

  // Unity's local "forward" is +Z — rotate that by the composed rotation
  return Vector3Transform((Vector3){ 0.0f, 0.0f, 1.0f }, rot);
}

void recalibrate_camera(Camera3D *camera)
{
  float aspect_adjustment = get_aspect_ratio_adjustment();

  camera->position.z    = Lerp(CAMERA_Z, CAMERA_Z_TABLET, aspect_adjustment);
  Vector3 forward       = unity_quarternion_to_forward((Vector3){ Lerp(CAMERA_ROT_X, CAMERA_ROT_X_TABLET, aspect_adjustment), 180.0f, 0.0f });
  camera->target        = Vector3Add(camera->position, Vector3Scale(forward, 10.0f));
  camera->fovy          = Lerp(50.0f, 65.0f, aspect_adjustment);
}

Vector3 project_to_camera_view(Camera3D camera, float distance, float offsetX, float offsetY)
{
    Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Vector3 right   = Vector3Normalize(Vector3CrossProduct(forward, camera.up));
    Vector3 up      = Vector3CrossProduct(right, forward);

    Vector3 center  = Vector3Add(camera.position, Vector3Scale(forward, distance));
    Vector3 result  = Vector3Add(center         , Vector3Scale(right, offsetX));
    result          = Vector3Add(result         , Vector3Scale(up, offsetY));

    return result;
}

static int camera_is_16_9(Camera *camera)
{
  return (1.77777779f - (1.0f * GetScreenWidth() / GetScreenHeight()) < 0.1f) ? 1 : 0;
}

Camera3D get_enwidened_camera(Camera3D camera, ValueChannel *enwidencamera_channel, float current_ms)
{
  Camera3D enwidened_camera = camera;
  Vector3 base_look_at = { 0, -5.5f, -20.0f };
  float z_pos = (camera_is_16_9(&camera) * 1.5f) + 3;
  float interpolation = value_channel_interpolate(enwidencamera_channel, current_ms);
  Vector3 enwiden_look_at = Vector3Scale((Vector3){ camera.position.x,
                                                    camera.position.y,
                                                    camera.position.z},
                                          interpolation / 2);
  enwidened_camera.position.y += interpolation * 4.5f;
  enwidened_camera.position.z += interpolation * z_pos;
  enwidened_camera.target = Vector3Add(base_look_at, enwiden_look_at);
  return enwidened_camera;
}
