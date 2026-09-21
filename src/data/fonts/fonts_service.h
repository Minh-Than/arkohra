#ifndef FONTS_SERVICE_H
#define FONTS_SERVICE_H

#include "raylib.h"
#include "data/custom_types/dynamic_list.h"

Font font_generate_sdf(char *font_file_path, int base_size, int *codepoints, int glyph_count);
bool font_file_has_codepoint(const char *path, int cp);
bool font_has_codepoint(Font f, int cp);
bool font_list_has_codepoint(List *font_list, int cp);
void font_add_missing_copepoints(List *font_list, const char* text, List *out);
bool font_is_mark_combining(int cp);
void font_add_string_to_codepoints(List *list, const char *text);

List  fonts_init(List *font_path_list, int base_size, int *codepoints, int glyph_count);
void  fonts_draw_text(List *font_list, const char *text, Vector2 pos, float size, float spacing, Color tint);
float fonts_measure_text(List *font_list, const char *text, float size, float spacing);

bool text_is_valid_int(const char *text);
int text_to_int_validated(const char *text, int fallback, int min, int max);
bool text_is_valid_decimal(const char *text);
float text_to_float_validated(const char *text, float fallback, float min, float max);
int count_decimals(const char *text);

#endif // FONTS_SERVICE_H
