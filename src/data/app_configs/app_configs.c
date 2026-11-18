#include "constants.h"
#include "raymath.h"
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

AppConfigs app_configs_init()
{
  AppConfigs configs = { 0 };
  rini_data rini_d = fetch_rini_config();
  
  if (!rini_key_exists(&rini_d, "screen_width"))   rini_set_value     (&rini_d, "screen_width", BASE_APP_WINDOW_WIDTH, "Screen width (960 - 3480px)");
  if (!rini_key_exists(&rini_d, "screen_ratio"))   rini_set_value     (&rini_d, "screen_ratio"  ,     0, "Window aspect ratio");
  if (!rini_key_exists(&rini_d, "scroll_speed"))   rini_set_float     (&rini_d, "scroll_speed"  ,  3.0f, "Chart speed (0.01 - 10.0)");
  if (!rini_key_exists(&rini_d, "music_volume"))   rini_set_float     (&rini_d, "music_volume"  ,  1.0f, "Chart music volume (0.0 - 1.0)");
  if (!rini_key_exists(&rini_d, "hit_volume"))     rini_set_float     (&rini_d, "hit_volume"    ,  0.2f, "Notes' sound effect volume (0.0 - 1.0)");
  if (!rini_key_exists(&rini_d, "ffmpeg_path"))    rini_set_value_text(&rini_d, "ffmpeg_path"   ,    "", "FFMPEG Executable Path");
  if (!rini_key_exists(&rini_d, "recent_project")) rini_set_value_text(&rini_d, "recent_project",    "", "Most recent chart path");
  if (!rini_key_exists(&rini_d, "colorblind"))     rini_set_value     (&rini_d, "colorblind"    ,     0, "Is colorblind");
  if (!rini_key_exists(&rini_d, "show_framerate")) rini_set_value     (&rini_d, "show_framerate",     0, "Show framerate");
  if (!rini_key_exists(&rini_d, "kohra_speed"))    rini_set_float     (&rini_d, "kohra_speed"   , 25.0f, "kohra speed");
  if (!rini_key_exists(&rini_d, "kohra_scale"))    rini_set_float     (&rini_d, "kohra_scale"   ,  1.0f, "kohra scale");

  configs.screen_width   =   (int)Clamp(rini_get_value_fallback(rini_d, "screen_width", BASE_APP_WINDOW_WIDTH), MINIMUM_SCREEN_WIDTH, MAXIMUM_SCREEN_WIDTH);
  configs.scroll_speed   =        Clamp(rini_get_float_fallback(rini_d, "scroll_speed"  ,  3.0f), MINIMUM_SCROLL_SPEED, MAXIMUM_SCROLL_SPEED);
  configs.music_volume   =        Clamp(rini_get_float_fallback(rini_d, "music_volume"  ,  1.0f), MINIMUM_AUDIO_VOLUME, MAXIMUM_AUDIO_VOLUME);
  configs.hit_volume     =        Clamp(rini_get_float_fallback(rini_d, "hit_volume"    ,  0.2f), MINIMUM_AUDIO_VOLUME, MAXIMUM_AUDIO_VOLUME);
  configs.colorblind     =        (bool)rini_get_value_fallback(rini_d, "colorblind"    ,     0);
  configs.show_framerate =        (bool)rini_get_value_fallback(rini_d, "show_framerate",     0);
  configs.kohra_speed    =        Clamp(rini_get_float_fallback(rini_d, "kohra_speed"   , 25.0f), MINIMUM_KOHRA_SPEED, MAXIMUM_KOHRA_SPEED);
  configs.kohra_scale    =        Clamp(rini_get_float_fallback(rini_d, "kohra_scale"   ,  1.0f), MINIMUM_KOHRA_SCALE, MAXIMUM_KOHRA_SCALE);
  configs.screen_ratio   = (AspectRatio)rini_get_value_fallback(rini_d, "screen_ratio", 0);
  if (!aspect_ratio_is_valid(configs.screen_ratio)) configs.screen_ratio = ASPECT_16_9;

  text_copy_bounded(configs.ffmpeg_path   , sizeof(configs.ffmpeg_path)   , rini_get_value_text_fallback(rini_d, "ffmpeg_path"   , ""));
  text_copy_bounded(configs.recent_project, sizeof(configs.recent_project), rini_get_value_text_fallback(rini_d, "recent_project", ""));

  rini_unload(&rini_d);
  app_configs_write_to_file(&configs);

  return configs;
}

void app_configs_write_to_file(AppConfigs *app_configs)
{
  rini_data rini_d = fetch_rini_config();
  rini_set_value     (&rini_d, "screen_ratio"  , app_configs->screen_ratio       , rini_get_value_description(rini_d, "screen_ratio"));
  rini_set_float     (&rini_d, "scroll_speed"  , app_configs->scroll_speed       , rini_get_value_description(rini_d, "scroll_speed"));
  rini_set_value     (&rini_d, "screen_width"  , app_configs->screen_width       , rini_get_value_description(rini_d, "screen_width"));
  rini_set_float     (&rini_d, "music_volume"  , app_configs->music_volume       , rini_get_value_description(rini_d, "music_volume"));
  rini_set_float     (&rini_d, "hit_volume"    , app_configs->hit_volume         , rini_get_value_description(rini_d, "hit_volume"));
  rini_set_value_text(&rini_d, "ffmpeg_path"   , app_configs->ffmpeg_path        , rini_get_value_description(rini_d, "ffmpeg_path"));
  rini_set_value_text(&rini_d, "recent_project", app_configs->recent_project     , rini_get_value_description(rini_d, "recent_project"));
  rini_set_value     (&rini_d, "colorblind"    , (int)app_configs->colorblind    , rini_get_value_description(rini_d, "colorblind"));
  rini_set_value     (&rini_d, "show_framerate", (int)app_configs->show_framerate, rini_get_value_description(rini_d, "show_framerate"));
  rini_set_float     (&rini_d, "kohra_speed"   , app_configs->kohra_speed        , rini_get_value_description(rini_d, "kohra_speed"));
  rini_set_float     (&rini_d, "kohra_scale"   , app_configs->kohra_scale        , rini_get_value_description(rini_d, "kohra_scale"));

  char app_dir [MAXPATHLEN];
  char ini_path[MAXPATHLEN];
  if (get_appdata_path("arkohra", app_dir, MAXPATHLEN) != 0) return;

  if (!dir_exists(app_dir)) {
    printf("Creating directory: %s\n", app_dir);
    if (MKDIR(app_dir) != 0) {
      printf("ERROR: Failed to create directory: %s\n", app_dir);
      return;
    }
  }

  snprintf(ini_path, MAXPATHLEN, "%s/config.ini", app_dir);
  rini_save(rini_d, ini_path);
  rini_unload(&rini_d);
}
