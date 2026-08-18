#ifndef COLOR_SERVICES_H
#define COLOR_SERVICES_H

#include "raylib.h"

Color from_rgba(const unsigned char rgba[4]);
Color from_hex(const char* hex);
#endif // COLOR_SERVICES_H
