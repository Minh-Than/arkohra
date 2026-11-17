#ifndef DRAWING_UTILS_H
#define DRAWING_UTILS_H

#include "raylib.h"
#include "rlgl.h"
#include "raymath.h" // IWYU pragma: export

void DrawKohraCube(Texture2D texture, Vector3 position,
                   float width, float height, float length,
                   Vector3 rotations, Vector3 axes,
                   Vector3 translation, Vector3 scale,
                   Color color);
void DrawConnector(Vector3 start, Vector3 end, float thick, Color color);
void DrawBeatline(float z_pos, float thick, Color color);

#endif // DRAWING_UTILS_H
