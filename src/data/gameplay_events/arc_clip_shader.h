#ifndef ARC_CLIP_SHADER_H
#define ARC_CLIP_SHADER_H

#include "raylib.h"

typedef struct
{
  Shader shader;
  int    clipZ_loc;
} ArcClipShader;

#endif // ARC_CLIP_SHADER_H
