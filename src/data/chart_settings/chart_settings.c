#include <stdio.h>
#include <_string.h>
#include <stdlib.h>
#include "chart_settings.h"
#include "color_services.h"
#include "constants.h"
#include "raylib.h"
#include "render/texture/single_line_type.h"
#include "render/texture/skin_side.h"

ChartSettings chart_settings_init(AppConfigs *app_configs)
{
  ChartSettings settings = { 0 };
  settings.base_bpm = 100.0f;
  settings.sync_base_bpm = false;
  settings.audio_offset = 0;
  settings.chart_constant = 0.0f;
  settings.scroll_speed = app_configs->scroll_speed;
  settings.skin_side = SK_LIGHT;
  settings.skin_track = SK_LIGHT;
  settings.sl_type = SL_NONE;
  TextCopy(settings.title, "Title");
  TextCopy(settings.composer, "Composer");
  TextCopy(settings.difficulty_color, color_rgba_to_hex(FTR_DIFF_COLOR));

  return settings;
}

void chart_settings_print(ChartSettings *chart_settings)
{
  printf("Chart Path: %s\n", chart_settings->chart_path);
  printf("Audio Path: %s\n", chart_settings->audio_path);
  printf("Jacket Path: %s\n", chart_settings->jacket_path);
  printf("Base BPM: %f\n", chart_settings->base_bpm);
  printf("BPM Text: %s\n", chart_settings->bpm_text);
  printf("Is BPM Sync: %d\n", chart_settings->sync_base_bpm);
  printf("Audio offset: %d\n", chart_settings->audio_offset);
  printf("Background Path: %s\n", chart_settings->background_path);
  printf("Title: %s\n", chart_settings->title);
  printf("Composer: %s\n", chart_settings->composer);
  printf("Charter: %s\n", chart_settings->charter);
  printf("Alias: %s\n", chart_settings->alias);
  printf("Illustrator: %s\n", chart_settings->illustrator);
  printf("Difficulty: %s\n", chart_settings->difficulty);
  printf("Chart Constant: %f\n", chart_settings->chart_constant);
  printf("Difficulty Color: %s\n", chart_settings->difficulty_color);
  printf("Scroll Speed: %f\n", chart_settings->scroll_speed);
  skin_side_print(chart_settings->skin_side);
  single_line_print(chart_settings->sl_type);
}
