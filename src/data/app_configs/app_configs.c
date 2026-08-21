#include "data/custom_types/custom_types.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "resource_util.h"
#include "app_config.h"

AppConfigs app_configs_init(rini_data *d)
{

  if (!rini_key_exists(d, "playfield_ratio"))   rini_set_value     (d, "playfield_ratio" ,    0, "Window aspect ratio");
  if (!rini_key_exists(d, "app_window_scale"))  rini_set_float     (d, "app_window_scale", 1.0f, "Window scaling for resizing purpose");
  if (!rini_key_exists(d, "scroll_speed"))      rini_set_float     (d, "scroll_speed"    , 3.0f, "Chart scrolling speed");
  if (!rini_key_exists(d, "music_volume"))      rini_set_float     (d, "music_volume"    , 1.0f, "Chart music volume (0.0 - 1.0)");
  if (!rini_key_exists(d, "hit_volume"))        rini_set_float     (d, "hit_volume"      , 0.2f, "Notes' sound effect volume (0.0 - 1.0)");
  if (!rini_key_exists(d, "ffmpeg_path"))       rini_set_value_text(d, "ffmpeg_path"     ,   "", "FFMPEG Executable Path");
  if (!rini_key_exists(d, "recent_project"))    rini_set_value_text(d, "recent_project"  ,   "", "Most recent chart path");

  AppConfigs configs = { 0 };
  configs.playfield_ratio   = (AspectRatio)rini_get_value_fallback(*d, "playfield_ratio" , 0);

  configs.app_window_scale = rini_get_float_fallback(*d, "app_window_scale", 1.0f);
  configs.scroll_speed     = rini_get_float_fallback(*d, "scroll_speed"    , 3.0f);
  configs.music_volume     = rini_get_float_fallback(*d, "music_volume"    , 1.0f);
  configs.hit_volume       = rini_get_float_fallback(*d, "hit_volume"      , 0.2f);
  TextCopy(configs.ffmpeg_path   , rini_get_value_text_fallback(*d, "ffmpeg_path"   , ""));
  TextCopy(configs.recent_project, rini_get_value_text_fallback(*d, "recent_project", ""));

  app_configs_write_to_file(&configs, d);
  configs.arc_clip_shader = LoadShader(
    TextFormat("resources/shaders/glsl%i/arc_clip.vs", 330),
    TextFormat("resources/shaders/glsl%i/arc_clip.fs", 330)
  );
  configs.clip_z_loc = GetShaderLocation(configs.arc_clip_shader, "clipZ");
  return configs;
}

void app_configs_write_to_file(AppConfigs *app_configs, rini_data *d)
{
  rini_set_value     (d, "playfield_ratio" , app_configs->playfield_ratio , rini_get_value_description(*d, "app_window_scale"));
  rini_set_float     (d, "app_window_scale", app_configs->app_window_scale, rini_get_value_description(*d, "app_window_scale"));
  rini_set_float     (d, "scroll_speed"    , app_configs->scroll_speed    , rini_get_value_description(*d, "scroll_speed"));
  rini_set_float     (d, "music_volume"    , app_configs->music_volume    , rini_get_value_description(*d, "music_volume"));
  rini_set_float     (d, "hit_volume"      , app_configs->hit_volume      , rini_get_value_description(*d, "hit_volume"));
  rini_set_value_text(d, "ffmpeg_path"     , app_configs->ffmpeg_path     , rini_get_value_description(*d, "ffmpeg_path"));
  rini_set_value_text(d, "recent_project"  , app_configs->recent_project  , rini_get_value_description(*d, "recent_project"));

  char app_dir [MAX_PATH_LEN];
  char ini_path[MAX_PATH_LEN];
  if (get_appdata_path("arckohra", app_dir, MAX_PATH_LEN) != 0) return;

  if (!dir_exists(app_dir)) {
    printf("Creating directory: %s\n", app_dir);
    if (MKDIR(app_dir) != 0) {
      printf("ERROR: Failed to create directory: %s\n", app_dir);
      return;
    }
  }

  snprintf(ini_path, MAX_PATH_LEN, "%s/config.ini", app_dir);
  rini_save(*d, ini_path);
}

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

FontServices font_services_init(int glsl)
{
  FontServices f    = { 0 };
  f.hud_sdf_shader = LoadShader(0, TextFormat("resources/shaders/glsl%i/sdf.fs", glsl));
  f.saira_regular  = GenerateSDF((char *)"resources/fonts/Saira-Regular.ttf", 45, NULL, 95);
  f.saira_medium   = GenerateSDF((char *)"resources/fonts/Saira-Medium.ttf", 45, NULL, 95);

  // f.noto_sans_tc_regular = GenerateSDF((char *)"resources/fonts/NotoSansTC-Regular.ttf", 45, NULL, 95);
  List hud_code_points; list_init(&hud_code_points, sizeof(int));
  for (int cp = 0x20; cp <= 0x7E; cp++) list_push(&hud_code_points, &cp);
  f.hud_notosans_tc_reg = GenerateSDF((char *)"resources/fonts/NotoSansTC-Regular.ttf", 45, (int *)hud_code_points.data, hud_code_points.size);
  list_free(&hud_code_points);

  return f;
}

void font_services_unload(FontServices *font_services)
{
  UnloadFont(font_services->saira_regular);
  UnloadFont(font_services->saira_medium);
  UnloadFont(font_services->hud_notosans_tc_reg);
  UnloadShader(font_services->hud_sdf_shader);
}

void DrawCubeTexture(Texture2D texture, Vector3 position, float width, float height, float length, Color color)
{
    float x = position.x;
    float y = position.y;
    float z = position.z;

    // Set desired texture to be enabled while drawing following vertex data
    rlSetTexture(texture.id);

    // Vertex data transformation can be defined with the commented lines,
    // but in this example we calculate the transformed vertex data directly when calling rlVertex3f()
    //rlPushMatrix();
        // NOTE: Transformation is applied in inverse order (scale -> rotate -> translate)
        //rlTranslatef(2.0f, 0.0f, 0.0f);
        //rlRotatef(45, 0, 1, 0);
        //rlScalef(2.0f, 2.0f, 2.0f);

        rlBegin(RL_QUADS);
            rlColor4ub(color.r, color.g, color.b, color.a);
            // Front Face
            rlNormal3f(0.0f, 0.0f, 1.0f);       // Normal Pointing Towards Viewer
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x - width/2, y - height/2, z + length/2);  // Bottom Left Of The Texture and Quad
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Right Of The Texture and Quad
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x + width/2, y + height/2, z + length/2);  // Top Right Of The Texture and Quad
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x - width/2, y + height/2, z + length/2);  // Top Left Of The Texture and Quad
            // Back Face
            rlNormal3f(0.0f, 0.0f, -1.0f);     // Normal Pointing Away From Viewer
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x - width/2, y - height/2, z - length/2);  // Bottom Right Of The Texture and Quad
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Right Of The Texture and Quad
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x + width/2, y + height/2, z - length/2);  // Top Left Of The Texture and Quad
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x + width/2, y - height/2, z - length/2);  // Bottom Left Of The Texture and Quad
            // Top Face
            rlNormal3f(0.0f, 1.0f, 0.0f);       // Normal Pointing Up
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Left Of The Texture and Quad
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x - width/2, y + height/2, z + length/2);  // Bottom Left Of The Texture and Quad
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x + width/2, y + height/2, z + length/2);  // Bottom Right Of The Texture and Quad
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x + width/2, y + height/2, z - length/2);  // Top Right Of The Texture and Quad
            // Bottom Face
            rlNormal3f(0.0f, -1.0f, 0.0f);     // Normal Pointing Down
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x - width/2, y - height/2, z - length/2);  // Top Right Of The Texture and Quad
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x + width/2, y - height/2, z - length/2);  // Top Left Of The Texture and Quad
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Left Of The Texture and Quad
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x - width/2, y - height/2, z + length/2);  // Bottom Right Of The Texture and Quad
            // Right face
            rlNormal3f(1.0f, 0.0f, 0.0f);       // Normal Pointing Right
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x + width/2, y - height/2, z - length/2);  // Bottom Right Of The Texture and Quad
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x + width/2, y + height/2, z - length/2);  // Top Right Of The Texture and Quad
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x + width/2, y + height/2, z + length/2);  // Top Left Of The Texture and Quad
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Left Of The Texture and Quad
            // Left Face
            rlNormal3f(-1.0f, 0.0f, 0.0f);    // Normal Pointing Left
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x - width/2, y - height/2, z - length/2);  // Bottom Left Of The Texture and Quad
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x - width/2, y - height/2, z + length/2);  // Bottom Right Of The Texture and Quad
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x - width/2, y + height/2, z + length/2);  // Top Right Of The Texture and Quad
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Left Of The Texture and Quad
        rlEnd();
    //rlPopMatrix();

    rlSetTexture(0);
}
void DrawConnectorLine(Vector3 start, Vector3 end, float thick, Color color)
{
  float half_thick = thick / 2;
  float dx = end.x - start.x;
  float dy = end.y - start.y;

  Vector2 left  = { -dy, dx }; left = Vector2Scale(Vector2Normalize(left), half_thick);
  Vector2 right = Vector2Scale(left, -1);

  rlBegin(RL_TRIANGLES);
    rlColor4ub(color.r, color.g, color.b, color.a);

    rlVertex3f(start.x + left.x , start.y + left.y , start.z);
    rlVertex3f(start.x + right.x, start.y + right.y, start.z);
    rlVertex3f(end.x   + left.x , end.y   + left.y , end.z  );

    rlVertex3f(start.x + right.x, start.y + right.y, start.z);
    rlVertex3f(end.x   + right.x, end.y   + right.y, end.z  );
    rlVertex3f(end.x   + left.x , end.y   + left.y , end.z  );
  rlEnd();
}
