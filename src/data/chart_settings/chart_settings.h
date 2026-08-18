#ifndef CHART_SETTINGS_H
#define CHART_SETTINGS_H

#include <stdbool.h>
#include "data/app_configs/app_config.h"
#include "render/texture/texture_service.h"
#include "render/texture/skin_side.h"

typedef struct {
  char  chart_path[256];
  char  audio_path[256];
  char  jacket_path[256];
  float base_bpm;
  char  bpm_text[32];
  bool  sync_base_bpm;
  int   audio_offset;
  char  background_path[256];
  char  title[256];
  char  composer[256];
  char  charter[128];
  char  alias[32];
  char  illustrator[32];
  char  difficulty[16];
  float chart_constant;
  char  difficulty_color[16];

  SkinSide skin_side, skin_track;
  float scroll_speed;
} ChartSettings;

ChartSettings chart_settings_init(AppConfigs *app_configs, TextureGroup *texture_group);
void chart_settings_print(ChartSettings *chart_settings);

#endif // CHART_SETTINGS_H
