#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <stdbool.h>
#include "data/custom_types/dynamic_list.h"
#include "rini.h"
#include "raylib.h"
#include "resource_util.h"

typedef enum 
{
  _16_9,
  _20_9,
  _18_9,
  _4_3,
  _3_2,
} AspectRatio;

typedef struct
{
  char          ffmpeg_path[MAX_PATH_LEN], recent_project[MAX_PATH_LEN];
  float         scroll_speed, music_volume, hit_volume;
  float         app_window_scale;
  AspectRatio   playfield_ratio;
} AppConfigs;

AppConfigs app_configs_init(rini_data *d);
void app_configs_write_to_file(AppConfigs *app_configs, rini_data *d);

typedef struct
{
  Shader hud_sdf_shader;
  Font saira_regular, saira_medium;
  Font hud_notosans_tc_reg;

} FontServices;

FontServices font_services_init(int glsl);
Font GenerateSDF(char *font_file_path, int base_size, int *codepoints, int glyph_count);
void font_services_unload(FontServices *font_services);
void DrawCubeTexture(Texture2D texture, Vector3 position, float width, float height, float length, Color color);
void DrawThickLine3D(Vector3 start, Vector3 end, float thick, Color color);
void AddCodepointRange(Font *font, const char *fontPath, int start, int stop);
void AddStringToCodepointList(List *list, const char *text);

#endif // APP_CONFIG_H
