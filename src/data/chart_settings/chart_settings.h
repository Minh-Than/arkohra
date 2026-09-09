#ifndef CHART_SETTINGS_H
#define CHART_SETTINGS_H

#include <stdbool.h>
#include <sys/param.h>
#include "data/app_configs/app_config.h"
#include "render/texture/single_line_type.h"
#include "render/texture/skin_side.h"

typedef struct {
  char  chart_path[MAXPATHLEN];
  char  audio_path[MAXPATHLEN];
  char  jacket_path[MAXPATHLEN];
  float base_bpm;
  char  bpm_text[256];
  bool  sync_base_bpm;
  int   audio_offset;
  char  background_path[MAXPATHLEN];
  char  title[256];
  char  composer[256];
  char  charter[256];
  char  alias[256];
  char  illustrator[256];
  char  difficulty[256];
  float chart_constant;
  char  difficulty_color[16];

  SkinSide skin_side, skin_track;
  SingleLineType sl_type;
  float scroll_speed;
} ChartSettings;

ChartSettings chart_settings_init(AppConfigs *app_configs);
void chart_settings_print(ChartSettings *chart_settings);

#endif // CHART_SETTINGS_H
