#include <stdio.h>
#include <stdlib.h>
#include "chart_settings.h"
#include "color_services.h"
#include "constants.h"
#include "raylib.h"
#include "cJSON.h"
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

bool chart_settings_load_from_project(ChartSettings *chart_settings, AppConfigs *app_configs, const char *project_file_path)
{
  const char *dir = GetDirectoryPath(project_file_path);
  char *project_json_text = LoadFileText(project_file_path);

  const cJSON *last_opened_chart_path = NULL;
  const cJSON *charts                 = NULL;
  const cJSON *chart_item             = NULL;
  cJSON *json = cJSON_Parse(project_json_text);
  if (json == NULL)
  {
    const char *error_ptr = cJSON_GetErrorPtr();
    if (error_ptr != NULL)
    {
      fprintf(stderr, "Error before %s\n", error_ptr);
    }
    cJSON_Delete(json);
    return false;
  }

  last_opened_chart_path = cJSON_GetObjectItemCaseSensitive(json, "lastOpenedChartPath");
  char aff_file_search[16]; TextCopy(aff_file_search, "2.aff");
  if (cJSON_IsString(last_opened_chart_path) && last_opened_chart_path->valuestring != NULL)
    text_copy_bounded(aff_file_search, 16, last_opened_chart_path->valuestring);

  charts = cJSON_GetObjectItemCaseSensitive(json, "charts");
  cJSON_ArrayForEach(chart_item, charts)
  {
    // Check for matching last opened chart path
    cJSON *chart_path = cJSON_GetObjectItemCaseSensitive(chart_item, "chartPath");
    if (!cJSON_IsString(chart_path) || !TextIsEqual(aff_file_search, chart_path->valuestring))
      continue;

    // Require audio path to continue parsing
    cJSON *audio_path = cJSON_GetObjectItemCaseSensitive(chart_item, "audioPath");
    if (!cJSON_IsString(audio_path)) { cJSON_Delete(json); return false; }

    // Reset chart settings
    *chart_settings = chart_settings_init(app_configs);

    text_copy_bounded(chart_settings->chart_path, MAXPATHLEN, TextFormat("%s/%s", dir, chart_path->valuestring));
    text_copy_bounded(chart_settings->audio_path, MAXPATHLEN, TextFormat("%s/%s", dir, audio_path->valuestring));

    cJSON *jacket_path = cJSON_GetObjectItemCaseSensitive(chart_item, "jacketPath");
    if (cJSON_IsString(jacket_path) && jacket_path->valuestring != NULL)
      text_copy_bounded(chart_settings->jacket_path, MAXPATHLEN, TextFormat("%s/%s", dir, jacket_path->valuestring));

    cJSON *background_path = cJSON_GetObjectItemCaseSensitive(chart_item, "backgroundPath");
    if (cJSON_IsString(background_path) && background_path->valuestring != NULL)
      text_copy_bounded(chart_settings->background_path, MAXPATHLEN, TextFormat("%s/%s", dir, background_path->valuestring));

    cJSON *base_bpm = cJSON_GetObjectItemCaseSensitive(chart_item, "baseBpm");
    if (cJSON_IsNumber(base_bpm)) chart_settings->base_bpm = (float)base_bpm->valuedouble;

    cJSON *bpm_text = cJSON_GetObjectItemCaseSensitive(chart_item, "bpmText");
    if (cJSON_IsString(bpm_text) && bpm_text->valuestring != NULL)
      text_copy_bounded(chart_settings->bpm_text, 256, bpm_text->valuestring);

    cJSON *sync_base_bpm = cJSON_GetObjectItemCaseSensitive(chart_item, "syncBaseBpm");
    if (cJSON_IsBool(sync_base_bpm)) chart_settings->sync_base_bpm = cJSON_IsTrue(sync_base_bpm);

    cJSON *title = cJSON_GetObjectItemCaseSensitive(chart_item, "title");
    if (cJSON_IsString(title) && title->valuestring != NULL)
      text_copy_bounded(chart_settings->title, 256, title->valuestring);

    cJSON *composer = cJSON_GetObjectItemCaseSensitive(chart_item, "composer");
    if (cJSON_IsString(composer) && composer->valuestring != NULL)
      text_copy_bounded(chart_settings->composer, 256, composer->valuestring);

    cJSON *alias = cJSON_GetObjectItemCaseSensitive(chart_item, "alias");
    if (cJSON_IsString(alias) && alias->valuestring != NULL)
      text_copy_bounded(chart_settings->alias, 256, alias->valuestring);

    cJSON *charter = cJSON_GetObjectItemCaseSensitive(chart_item, "charter");
    if (cJSON_IsString(charter) && charter->valuestring != NULL)
      text_copy_bounded(chart_settings->charter, 256, charter->valuestring);

    cJSON *illustrator = cJSON_GetObjectItemCaseSensitive(chart_item, "illustrator");
    if (cJSON_IsString(illustrator) && illustrator->valuestring != NULL)
      text_copy_bounded(chart_settings->illustrator, 256, illustrator->valuestring);

    cJSON *difficulty = cJSON_GetObjectItemCaseSensitive(chart_item, "difficulty");
    if (cJSON_IsString(difficulty) && difficulty->valuestring != NULL)
      text_copy_bounded(chart_settings->difficulty, 256, difficulty->valuestring);

    cJSON *chart_constant = cJSON_GetObjectItemCaseSensitive(chart_item, "chartConstant");
    if (cJSON_IsNumber(chart_constant)) chart_settings->chart_constant = (float)chart_constant->valuedouble;

    cJSON *difficulty_color = cJSON_GetObjectItemCaseSensitive(chart_item, "difficultyColor");
    if (cJSON_IsString(difficulty_color) && difficulty_color->valuestring != NULL)
      text_copy_bounded(chart_settings->difficulty_color, 256, difficulty_color->valuestring);

    cJSON *skin = cJSON_GetObjectItemCaseSensitive(chart_item, "skin");
    if (cJSON_IsObject(skin)){
      cJSON *side = cJSON_GetObjectItemCaseSensitive(skin, "side");
      if (cJSON_IsString(side) && side->valuestring != NULL)
      {
        chart_settings->skin_track = skin_side_get_by_string(side->valuestring);
        chart_settings->skin_side  = skin_side_get_by_string(side->valuestring);
      }

      cJSON *track = cJSON_GetObjectItemCaseSensitive(skin, "track");
      if (cJSON_IsString(track) && track->valuestring != NULL)
        chart_settings->skin_track = skin_side_get_by_string(track->valuestring);

      cJSON *single_line= cJSON_GetObjectItemCaseSensitive(skin, "singleLine");
      if (cJSON_IsString(single_line) && single_line->valuestring != NULL)
        chart_settings->sl_type = single_line_get_by_string(single_line->valuestring);
    }

    cJSON_Delete(json); return true;
  }
  cJSON_Delete(json); return false;
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
