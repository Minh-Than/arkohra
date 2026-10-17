#ifndef ARC_SHADER_H
#define ARC_SHADER_H

#include "raylib.h"

typedef struct
{
  Shader shader;
  int worldX1_loc, worldY1_loc, worldX2_loc, worldY2_loc;
  int lengthDelta_loc, scale_loc;
  int shouldClip_loc, negativeBPM_loc;
  int tintLow_loc, tintHigh_loc;
} ArcShader;

#endif // ARC_SHADER_H
