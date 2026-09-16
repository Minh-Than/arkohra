#include "data/custom_types/dynamic_list.h"
#include "external/stb_truetype.h"
#include "fonts_service.h"

Font font_generate_sdf(char *font_file_path, int base_size, int *codepoints, int glyph_count)
{
  int fileSize = 0;
  unsigned char *font_file_data = LoadFileData(font_file_path, &fileSize);
  Font font_sdf = { 0 };
  font_sdf.baseSize   = base_size;
  font_sdf.glyphCount = glyph_count;
  font_sdf.glyphs     = LoadFontData(font_file_data, fileSize,
                                     font_sdf.baseSize,
                                     codepoints, glyph_count,
                                     FONT_SDF, &font_sdf.glyphCount);

  Image atlas = GenImageFontAtlas(font_sdf.glyphs, &font_sdf.recs,
                                  font_sdf.glyphCount,
                                  font_sdf.baseSize, 0, 1);
  font_sdf.texture = LoadTextureFromImage(atlas);
  UnloadImage(atlas);
  UnloadFileData(font_file_data);
  SetTextureFilter(font_sdf.texture, TEXTURE_FILTER_BILINEAR);
  return font_sdf;
}

// Ask the FONT FILE what it contains.
// Only this one can see through the .notdef fallback, so it must read the TTF.
bool font_file_has_glyph(const char *path, int cp)
{
  int size = 0;
  unsigned char *data = LoadFileData(path, &size);
  if (data == NULL) return false;

  stbtt_fontinfo info;
  bool has = false;
  if (stbtt_InitFont(&info, data, stbtt_GetFontOffsetForIndex(data, 0)))
    has = (stbtt_FindGlyphIndex(&info, cp) != 0);

  UnloadFileData(data);
  return has;
}

// Ask the BAKED FONT if this codepoint is in its atlas.
bool font_has_glyph(Font f, int cp)
{
  int index = GetGlyphIndex(f, cp);
  return (index >= 0 && index < f.glyphCount && f.glyphs[index].value == cp);
}

bool font_is_mark_combining(int cp)
{
  return (cp >= 0x0300 && cp <= 0x036F)
      || (cp >= 0x1AB0 && cp <= 0x1AFF)
      || (cp >= 0x1DC0 && cp <= 0x1DFF)
      || (cp >= 0x20D0 && cp <= 0x20F0)
      || (cp >= 0xFE20 && cp <= 0xFE2F);
}

void font_add_string_to_codepoints(List *list, const char *text)
{
  if (!text) return;
  int byteOffset = 0;
  int codepointSize = 0;
  while (text[byteOffset] != '\0')
  {
    int cp = GetCodepoint(&text[byteOffset], &codepointSize);
    if (cp != 0)
    {
      // Check for duplicates
      bool exists = false;
      for (int i = 0; i < list->size; i++) {
        int *cp_i = (int *)list_get(list, i);
        if (*cp_i == cp) { exists = true; break; }
      }
      if (!exists) list_push(list, &cp);
    }
    byteOffset += codepointSize;
  }
}

typedef struct {
  unsigned char *data;
  stbtt_fontinfo info;
  bool valid;
} FontProbe;

static FontProbe font_probe_load(const char *path)
{
  FontProbe p = { 0 };
  int size = 0;
  p.data = LoadFileData(path, &size);
  if (p.data)
    p.valid = stbtt_InitFont(&p.info, p.data, stbtt_GetFontOffsetForIndex(p.data, 0));
  return p;
}

static void font_probe_free(FontProbe *p)
{
  if (p->data) UnloadFileData(p->data);
  p->data = NULL;
  p->valid = false;
}

static bool font_probe_has_glyph(const FontProbe *p, int cp)
{
  return p->valid && (stbtt_FindGlyphIndex(&p->info, cp) != 0);
}

List fonts_init(List *font_path_list, int base_size, int *codepoints, int glyph_count)
{
  List result; list_init(&result, sizeof(Font));

  for (int i = 0; i < font_path_list->size; i++)
  {
    char *path = *(char **)list_get(font_path_list, i);
    List codepoint_entry; list_init(&codepoint_entry, sizeof(int));

    // Populate codepoints
    // (probe the font info once)
    FontProbe prb = font_probe_load(path);
    for (int j = 0; j < glyph_count; j++)
    {
      int codepoint = codepoints[j];
      if (font_probe_has_glyph(&prb, codepoint))
        list_push(&codepoint_entry, &codepoint);
    }
    font_probe_free(&prb);

    // Generate final SDF font
    if (codepoint_entry.size == 0) continue;
    Font entry_sdf_font = font_generate_sdf(path, base_size, (int *)codepoint_entry.data, codepoint_entry.size);
    list_push(&result, &entry_sdf_font);
    list_free(&codepoint_entry);
  }

  return result;
}

void fonts_draw_text(List *font_list, const char *text, Vector2 pos, float size, float spacing, Color tint)
{
  if (font_list->data == NULL || font_list->size == 0) return;

  float base_x = pos.x;
  int offset = 0, cpsize = 0;
  while (text[offset] != '\0')
  {
    int cp = GetCodepoint(&text[offset], &cpsize);

    Font *use = (Font *)list_get(font_list, 0);
    for (int i = 0; i < font_list->size; i++)
    {
      Font *f = (Font *)list_get(font_list, i);
      if (font_has_glyph(*f, cp)) { use = f; break; }
    }

    if (font_is_mark_combining(cp)) // If the mark chained rather than stacked (normal case)
      DrawTextCodepoint(*use, cp, (Vector2){ base_x, pos.y }, size, tint);
    else
    {
      DrawTextCodepoint(*use, cp, pos, size, tint);

      int index = GetGlyphIndex(*use, cp);
      float scale = size / (float)use->baseSize;
      if (index < use->glyphCount && use->glyphs[index].value == cp && use->glyphs[index].advanceX > 0)
        pos.x += use->glyphs[index].advanceX * scale + spacing;
      else
        pos.x += size * 0.5f + spacing; // No glyph at all for this: abitrarily set custom width
      base_x = pos.x;
    }

    offset += cpsize;
  }
}

float fonts_measure_text(List *font_list, const char *text, float size, float spacing)
{
  if (font_list->data == NULL || font_list->size == 0) return 0;

  float width = 0;
  int offset = 0, cpsize = 0;
  while (text[offset] != '\0')
  {
    int cp = GetCodepoint(&text[offset], &cpsize);

    Font *use = (Font *)list_get(font_list, 0);
    for (int i = 0; i < font_list->size; i++)
    {
      Font *f = (Font *)list_get(font_list, i);
      if (font_has_glyph(*f, cp)) { use = f; break; }
    }

    if (!font_is_mark_combining(cp)) // If the mark chained rather than stacked (normal case)
    {
      int index = GetGlyphIndex(*use, cp);
      float scale = size / (float)use->baseSize;
      if (index < use->glyphCount && use->glyphs[index].value == cp && use->glyphs[index].advanceX > 0)
        width += use->glyphs[index].advanceX * scale + spacing;
      else
        width += size * 0.5f + spacing; // No glyph at all for this: abitrarily set custom width
    }

    offset += cpsize;
  }
  return width;
}
