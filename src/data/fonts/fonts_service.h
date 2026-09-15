#ifndef FONTS_SERVICE_H
#define FONTS_SERVICE_H

#include "raylib.h"
#include "data/custom_types/dynamic_list.h"

Font GenerateSDF(char *font_file_path, int base_size, int *codepoints, int glyph_count);
void AddStringToCodepointList(List *list, const char *text);

#endif // FONTS_SERVICE_H
