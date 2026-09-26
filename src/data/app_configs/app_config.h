#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <stdbool.h>
#include <sys/param.h>
#include "rini.h"
#include "resource_util.h"

typedef enum {
  ASPECT_16_9 = 0,
  ASPECT_20_9 = 1,
  ASPECT_18_9 = 2,
  ASPECT_4_3  = 3,
  ASPECT_3_2  = 4,
} AspectRatio;

float aspect_ratio_get_height(float width, AspectRatio ratio);
float aspect_ratio_get_width(float height, AspectRatio ratio);

typedef struct
{
  char          ffmpeg_path[MAXPATHLEN], recent_project[MAXPATHLEN];
  float         scroll_speed, music_volume, hit_volume;
  AspectRatio   playfield_ratio;
  bool          kohra, colorblind;
} AppConfigs;

AppConfigs app_configs_init(rini_data *d);
void app_configs_write_to_file(AppConfigs *app_configs, rini_data *d);

#endif // APP_CONFIG_H
