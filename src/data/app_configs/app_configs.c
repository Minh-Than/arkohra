#include "raylib.h"
#include "rlgl.h"
#include "resource_util.h"
#include "app_config.h"

static bool aspect_ratio_is_valid(AspectRatio ratio)
{
  return ratio >= ASPECT_16_9 && ratio <= ASPECT_3_2;
}

float aspect_ratio_get_height(float width, AspectRatio ratio)
{
  switch(ratio)
  {
    case ASPECT_16_9: return (float)width * 9.0f / 16.0f;
    case ASPECT_20_9: return (float)width * 9.0f / 20.0f;
    case ASPECT_18_9: return (float)width * 9.0f / 18.0f;
    case ASPECT_4_3 : return (float)width * 3.0f / 4.0f;
    case ASPECT_3_2 : return (float)width * 2.0f / 3.0f;
  }
}

float aspect_ratio_get_width(float height, AspectRatio ratio)
{
  switch(ratio)
  {
    case ASPECT_16_9: return (float)height * 16.0f / 9.0f;
    case ASPECT_20_9: return (float)height * 20.0f / 9.0f;
    case ASPECT_18_9: return (float)height * 18.0f / 9.0f;
    case ASPECT_4_3 : return (float)height * 4.0f / 3.0f;
    case ASPECT_3_2 : return (float)height * 3.0f / 2.0f;
  }
}

AppConfigs app_configs_init(rini_data *d)
{
  if (!rini_key_exists(d, "playfield_ratio"))   rini_set_value     (d, "playfield_ratio" ,    0, "Window aspect ratio");
  if (!rini_key_exists(d, "scroll_speed"))      rini_set_float     (d, "scroll_speed"    , 3.0f, "Chart scrolling speed");
  if (!rini_key_exists(d, "music_volume"))      rini_set_float     (d, "music_volume"    , 1.0f, "Chart music volume (0.0 - 1.0)");
  if (!rini_key_exists(d, "hit_volume"))        rini_set_float     (d, "hit_volume"      , 0.2f, "Notes' sound effect volume (0.0 - 1.0)");
  if (!rini_key_exists(d, "ffmpeg_path"))       rini_set_value_text(d, "ffmpeg_path"     ,   "", "FFMPEG Executable Path");
  if (!rini_key_exists(d, "recent_project"))    rini_set_value_text(d, "recent_project"  ,   "", "Most recent chart path");
  if (!rini_key_exists(d, "colorblind"))        rini_set_value     (d, "colorblind"      ,    0, "Is colorblind");

  AppConfigs configs = { 0 };
  configs.playfield_ratio   = (AspectRatio)rini_get_value_fallback(*d, "playfield_ratio" , 0);
  if (!aspect_ratio_is_valid(configs.playfield_ratio)) configs.playfield_ratio = ASPECT_16_9;

  configs.scroll_speed     = rini_get_float_fallback(*d, "scroll_speed"    , 3.0f);
  configs.music_volume     = rini_get_float_fallback(*d, "music_volume"    , 1.0f);
  configs.hit_volume       = rini_get_float_fallback(*d, "hit_volume"      , 0.2f);
  TextCopy(configs.ffmpeg_path   , rini_get_value_text_fallback(*d, "ffmpeg_path"   , ""));
  TextCopy(configs.recent_project, rini_get_value_text_fallback(*d, "recent_project", ""));
  configs.colorblind = (bool)rini_get_value_fallback(*d, "colorblind" , 0);

  app_configs_write_to_file(&configs, d);
  return configs;
}

void app_configs_write_to_file(AppConfigs *app_configs, rini_data *d)
{
  rini_set_value     (d, "playfield_ratio" , app_configs->playfield_ratio , rini_get_value_description(*d, "app_window_scale"));
  rini_set_float     (d, "scroll_speed"    , app_configs->scroll_speed    , rini_get_value_description(*d, "scroll_speed"));
  rini_set_float     (d, "music_volume"    , app_configs->music_volume    , rini_get_value_description(*d, "music_volume"));
  rini_set_float     (d, "hit_volume"      , app_configs->hit_volume      , rini_get_value_description(*d, "hit_volume"));
  rini_set_value_text(d, "ffmpeg_path"     , app_configs->ffmpeg_path     , rini_get_value_description(*d, "ffmpeg_path"));
  rini_set_value_text(d, "recent_project"  , app_configs->recent_project  , rini_get_value_description(*d, "recent_project"));
  rini_set_value     (d, "colorblind"      , (int)app_configs->colorblind , rini_get_value_description(*d, "colorblind"));

  char app_dir [MAXPATHLEN];
  char ini_path[MAXPATHLEN];
  if (get_appdata_path("arckohra", app_dir, MAXPATHLEN) != 0) return;

  if (!dir_exists(app_dir)) {
    printf("Creating directory: %s\n", app_dir);
    if (MKDIR(app_dir) != 0) {
      printf("ERROR: Failed to create directory: %s\n", app_dir);
      return;
    }
  }

  snprintf(ini_path, MAXPATHLEN, "%s/config.ini", app_dir);
  rini_save(*d, ini_path);
}
