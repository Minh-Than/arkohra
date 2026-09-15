#include "fonts_service.h"

Font GenerateSDF(char *font_file_path, int base_size, int *codepoints, int glyph_count)
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

void AddStringToCodepointList(List *list, const char *text)
{
  if (!text) return;
  int byteOffset = 0;
  int codepointSize = 0;
  while (text[byteOffset] != '\0')
  {
    int cp = GetCodepoint(&text[byteOffset], &codepointSize);
    if (cp != 0)
    {
      // Optional: check for duplicates before adding to keep list unique
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
