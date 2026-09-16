#ifndef FONTS_SERVICE_H
#define FONTS_SERVICE_H

#define MAX_FONTS 2
#include "raylib.h"
#include "data/custom_types/dynamic_list.h"

Font font_generate_sdf(char *font_file_path, int base_size, int *codepoints, int glyph_count);
bool font_file_has_glyph(const char *path, int cp);
bool font_has_glyph(Font f, int cp);
bool font_is_mark_combining(int cp);
void font_add_string_to_codepoints(List *list, const char *text);

typedef struct {
  Font fonts[MAX_FONTS];
  int count;
} FontChain;

FontChain font_chain_init(const char *primary_path, const char *fallback_path,
                        int base_size, int *codepoints, int glyph_count);
void font_chain_draw(FontChain *chain, const char *text,
                   Vector2 pos, float size, float spacing, Color tint);
float font_chain_measure(FontChain *chain, const char *text, float size, float spacing);

#endif // FONTS_SERVICE_H
