#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <stdbool.h>
#include <sys/param.h>
#include "rini.h"
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
  char          ffmpeg_path[MAXPATHLEN], recent_project[MAXPATHLEN];
  float         scroll_speed, music_volume, hit_volume;
  float         app_window_scale;
  AspectRatio   playfield_ratio;
  bool          kohra;
} AppConfigs;

AppConfigs app_configs_init(rini_data *d);
void app_configs_write_to_file(AppConfigs *app_configs, rini_data *d);

#endif // APP_CONFIG_H
