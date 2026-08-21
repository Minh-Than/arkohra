#ifndef COLOR_SERVICES_H
#define COLOR_SERVICES_H

#include "raylib.h"

Color color_from_rgba(const unsigned char rgba[4]);
Color color_from_hex(const char* hex);
const char* color_rgba_to_hex(const unsigned char rgba[4]);
#endif // COLOR_SERVICES_H
