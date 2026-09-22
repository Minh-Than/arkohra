#include "raylib.h"
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
