#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <stdbool.h>
#include "color_services.h"
#include "raylib.h"

Color color_from_rgba(const unsigned char rgba[4]) { return (Color){rgba[0], rgba[1], rgba[2], rgba[3]}; }

Color color_from_hex(const char* hex)
{
  Color color = (Color){ 0, 0, 0, 255 };

  if(hex[0] == '#') hex++;

  unsigned int value = (unsigned int)strtol(hex, NULL, 16);
  int len = TextLength(hex);

  if (len == 8)
  {
    color.r = (value >> 24) & 0xFF;
    color.g = (value >> 16) & 0xFF;
    color.b = (value >>  8) & 0xFF;
    color.a = value & 0xFF;
  } else if (len == 6)
  {
    color.r = (value >> 16) & 0xFF;
    color.g = (value >>  8) & 0xFF;
    color.b = value & 0xFF;
  } else if (len == 4)
  {
    color.r = (value >> 12 & 0xF) * 0x11;  // 0xF -> 0xFF
    color.g = (value >>  8 & 0xF) * 0x11;  // 0x8 -> 0x88
    color.b = (value >>  4 & 0xF) * 0x11;  // 0xC -> 0xCC
    color.a = (value       & 0xF) * 0x11;
  } else if (len == 3)
  {
    color.r = (value >> 8 & 0xF) * 0x11;
    color.g = (value >> 4 & 0xF) * 0x11;
    color.b = (value      & 0xF) * 0x11;
  }

  return color;
}

const char* color_rgba_to_hex(const unsigned char rgba[4])
{
  return rgba[3] == 255 ? TextFormat("#%x%x%x%x", rgba[0], rgba[1], rgba[2], rgba[3])
                        : TextFormat("#%x%x%x", rgba[0], rgba[1], rgba[2]);
}
