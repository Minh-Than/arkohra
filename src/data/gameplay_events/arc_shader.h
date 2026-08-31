#ifndef ARC_SHADER_H
#define ARC_SHADER_H

#include "raylib.h"

typedef struct
{
  Shader shader;
  int isVoid_loc, shouldClip_loc, negativeBPM_loc;
  int tintLow_loc, tintHigh_loc;
} ArcShader;

#endif // ARC_SHADER_H
