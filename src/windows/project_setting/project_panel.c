#include "raygui.h"
#include "raymath.h"
#include "constants.h"
#include "data/fonts/fonts_service.h"
#include "project_panel.h"

ProjectPanel project_panel_init()
{
  ProjectPanel data = { 0 };
  data.scroll = Vector2Zero();
  data.scroll_rect = (Rectangle){ 0, 0 };

  data.title = data.composer = data.illustrator = data.charter = data.diff_text = data.alias  = rgui_textinput_init("");

  data.bpm_text = data.search_tags = rgui_textinput_init("");
  data.base_bpm = rgui_floatinput_init("0.0000");
  data.cc = rgui_floatinput_init("0.00");
  data.chart_offset = data.judge_density = data.preview_from = rgui_intinput_init("0");
  data.preview_to = rgui_intinput_init("10000");

  data.audio = data.jacket = data.background = data.bg_video = rgui_fileinput_init("");
  list_push(&data.audio.extensions, ".ogg");
  list_push(&data.jacket.extensions, ".png");   list_push(&data.background.extensions, ".png");
  list_push(&data.jacket.extensions, ".jpeg");  list_push(&data.background.extensions, ".jpeg");
  list_push(&data.jacket.extensions, ".jpg");   list_push(&data.background.extensions, ".jpg");
  list_push(&data.bg_video.extensions, ".mp4");

  return data;
}

void project_panel_draw(WindowInst* window_inst, ProjectPanel *project_panel,
                        AppConfigs *app_configs, RenderContext *render_ctx,
                        Shader sdf_shader, List *font_fallbacks)
{
  int content_x = window_inst->x + 8;
  int toggle_y  = (int)window_inst->y + 32;
  int scroll_y  = toggle_y + 32;
  int scroll_w  = (int)window_inst->width  - 16;
  int scroll_h  = (int)window_inst->height - (scroll_y - (int)window_inst->y) - 8;

  BeginShaderMode(sdf_shader);

  int info_panel_h = 200;
  int gameplay_panel_h = 230;
  int files_panel_h = 140;

  int input_field_h = 24;
  int input_field_gap = 30;
  int total_content_h = info_panel_h + gameplay_panel_h + files_panel_h + 12*3 + 8;
  GuiScrollPanel((Rectangle){content_x, scroll_y, scroll_w, scroll_h},
                 NULL, (Rectangle){0, 0, scroll_w, total_content_h},
                 &project_panel->scroll, &project_panel->scroll_rect);

  BeginScissorMode(project_panel->scroll_rect.x, project_panel->scroll_rect.y, project_panel->scroll_rect.width, project_panel->scroll_rect.height);

  // Information
  int info_panel_x = content_x + 8    + (int)project_panel->scroll.x;
  int info_panel_y = scroll_y  + 12*1 + (int)project_panel->scroll.y;
  int info_panel_w = scroll_w  - 28;
  int label_xs = 96;
  GuiGroupBox ((Rectangle){info_panel_x, info_panel_y, info_panel_w, info_panel_h}, "[ INFORMATION ]");

  int info_field_x = info_panel_x + 8;
  int info_field_y = info_panel_y + 16;
  int gameplay_input_x = info_field_x + label_xs;

  char *info_labels[] = { "Title", "Composer", "Illustrator", "Charter", "Difficulty Name", "Alias" };
  char *info_values[] = {
    project_panel->title.text, project_panel->composer.text,
    project_panel->illustrator.text, project_panel->charter.text,
    project_panel->diff_text.text, project_panel->alias.text
  };
  char *info_snapshots[] = {
    project_panel->title.snapshot, project_panel->composer.snapshot,
    project_panel->illustrator.snapshot, project_panel->charter.snapshot,
    project_panel->diff_text.snapshot, project_panel->alias.snapshot
  };
  bool *info_edit[] = {
    &project_panel->title.edit, &project_panel->composer.edit,
    &project_panel->illustrator.edit, &project_panel->charter.edit,
    &project_panel->diff_text.edit, &project_panel->alias.edit
  };
  for (int i = 0; i < 6; i++)
  {
    int row_y = info_field_y + input_field_gap * i;
    GuiLabel((Rectangle){ info_field_x, row_y, label_xs, input_field_h }, info_labels[i]);

    bool wasEditing = *info_edit[i];
    if (rgui_textinput_draw_font((Rectangle){ info_field_x + label_xs, row_y, info_panel_w - 16 - label_xs, input_field_h },
                        info_values[i], RGUI_INPUT_TEXT_CAP, 20, 0.4f, wasEditing, font_fallbacks))
    {
      *info_edit[i] = !wasEditing;
      if (!wasEditing)
        snprintf(info_snapshots[i], RGUI_INPUT_TEXT_CAP, "%s", info_values[i]);
      else if (strcmp(info_snapshots[i], info_values[i]) != 0)
        project_panel->is_text_edited = true; // RELOAD FONT
    }
    // rgui_textinput_draw_font() ends scissor mode
    BeginScissorMode(project_panel->scroll_rect.x, project_panel->scroll_rect.y, project_panel->scroll_rect.width, project_panel->scroll_rect.height);
  }

  // Gameplay
  int gameplay_panel_x = content_x + 8    + (int)project_panel->scroll.x;
  int gameplay_panel_y = scroll_y  + 12*2 + (int)project_panel->scroll.y + info_panel_h;
  int gameplay_panel_w = scroll_w  - 28;
  GuiGroupBox ((Rectangle){gameplay_panel_x, gameplay_panel_y, gameplay_panel_w, gameplay_panel_h}, "[ GAMEPLAY ]");

  int gameplay_field_x = gameplay_panel_x + 8;
  int gameplay_field_y = gameplay_panel_y + 16;

  Rectangle gameplay_label_rect = { gameplay_field_x, gameplay_field_y, label_xs, input_field_h };
  Rectangle gameplay_field_rect = { gameplay_input_x, gameplay_field_y, gameplay_panel_w - 16 - label_xs, input_field_h };

  int is_sync_sub_x = 70;
  GuiLabel(gameplay_label_rect, "Base BPM");
  int base_bpm_checkbox_x = gameplay_input_x + info_panel_w - 16 - label_xs - is_sync_sub_x + 8;
  Rectangle base_bpm_field_rect = { gameplay_input_x, gameplay_field_y, gameplay_panel_w - 16 - label_xs - is_sync_sub_x - 8, input_field_h };
  rgui_floatinput_textbox(&project_panel->base_bpm, base_bpm_field_rect, PROJ_BASE_BPM_DECIMALS);
  GuiCheckBox((Rectangle){base_bpm_checkbox_x, gameplay_field_y + input_field_gap*0, input_field_h, input_field_h},
              "Sync", &project_panel->is_sync);

  gameplay_label_rect.y += input_field_gap;
  gameplay_field_rect.y += input_field_gap;
  GuiLabel(gameplay_label_rect, "BPM Text");
  bool bpm_text_curr_edit = !project_panel->bpm_text.edit;
  if (rgui_textinput_draw_font(gameplay_field_rect, project_panel->bpm_text.text, RGUI_INPUT_TEXT_CAP,
                               20, 1.0f, project_panel->bpm_text.edit, font_fallbacks))
  {
    project_panel->bpm_text.edit = !project_panel->bpm_text.edit;
    if (!bpm_text_curr_edit)
      snprintf(project_panel->bpm_text.snapshot, RGUI_INPUT_TEXT_CAP, "%s", project_panel->bpm_text.text);
    else if (strcmp(project_panel->bpm_text.snapshot, project_panel->bpm_text.text) != 0)
      project_panel->is_text_edited = true; // RELOAD FONT
  }

  gameplay_label_rect.y += input_field_gap;
  gameplay_field_rect.y += input_field_gap;
  GuiLabel(gameplay_label_rect, "Chart Offset");
  rgui_intinput_textbox(&project_panel->chart_offset, gameplay_field_rect);

  gameplay_label_rect.y += input_field_gap;
  gameplay_field_rect.y += input_field_gap;
  GuiLabel(gameplay_label_rect, "Judge Density");
  rgui_intinput_textbox(&project_panel->judge_density, gameplay_field_rect);

  gameplay_label_rect.y += input_field_gap;
  gameplay_field_rect.y += input_field_gap;
  GuiLabel(gameplay_label_rect, "Chart Constant");
  rgui_floatinput_textbox(&project_panel->cc, gameplay_field_rect, PROJ_FLOAT_DECIMALS);

  gameplay_label_rect.y += input_field_gap;
  gameplay_field_rect.y += input_field_gap;
  GuiLabel(gameplay_label_rect, "Preview Segment");
  rgui_intinput_textbox(&project_panel->preview_from,
                        (Rectangle){ gameplay_input_x, gameplay_field_y + input_field_gap*5,
                                     ((float)info_panel_w / 2) - label_xs + 38, input_field_h });
  rgui_intinput_textbox(&project_panel->preview_to,
                        (Rectangle){ gameplay_input_x + ((float)info_panel_w / 2) - label_xs + 44, gameplay_field_y + input_field_gap*5,
                                     ((float)info_panel_w / 2) - label_xs + 36, input_field_h });

  gameplay_label_rect.y += input_field_gap;
  gameplay_field_rect.y += input_field_gap;
  bool search_tags_curr_edit = !project_panel->bpm_text.edit;
  GuiLabel(gameplay_label_rect, "Search Tag");
  if (rgui_textinput_draw_font(gameplay_field_rect, project_panel->search_tags.text, RGUI_INPUT_TEXT_CAP, 20, 0.4f, project_panel->search_tags.edit, font_fallbacks))
  {
    project_panel->search_tags.edit = !project_panel->search_tags.edit;
    if (!search_tags_curr_edit)
      snprintf(project_panel->search_tags.snapshot, RGUI_INPUT_TEXT_CAP, "%s", project_panel->search_tags.text);
    else if (strcmp(project_panel->search_tags.snapshot, project_panel->search_tags.text) != 0)
      project_panel->is_text_edited = true; // RELOAD FONT
  }

  // Files
  int files_panel_x = content_x + 8    + (int)project_panel->scroll.x;
  int files_panel_y = scroll_y  + 12*3 + (int)project_panel->scroll.y + info_panel_h + gameplay_panel_h;
  int files_panel_w = scroll_w  - 28;
  GuiGroupBox ((Rectangle){files_panel_x, files_panel_y, files_panel_w, files_panel_h}, "[ FILES ]");

  int files_field_x = files_panel_x + 8;
  int files_field_y = files_panel_y + 16;
  int files_input_x = info_field_x + label_xs;

  Rectangle files_label_rect = { files_field_x, files_field_y, label_xs, input_field_h };
  Rectangle files_field_rect = { files_input_x, files_field_y, files_panel_w - 16 - label_xs, input_field_h };

  GuiLabel(files_label_rect, "Audio");
  rgui_fileinput_button(&project_panel->audio, files_field_rect);

  files_label_rect.y += input_field_gap;
  files_field_rect.y += input_field_gap;
  GuiLabel(files_label_rect, "Jacket Art");
  rgui_fileinput_button(&project_panel->jacket, files_field_rect);

  files_label_rect.y += input_field_gap;
  files_field_rect.y += input_field_gap;
  GuiLabel(files_label_rect, "Background");
  rgui_fileinput_button(&project_panel->background, files_field_rect);

  files_label_rect.y += input_field_gap;
  files_field_rect.y += input_field_gap;
  GuiLabel(files_label_rect, "Video");
  rgui_fileinput_button(&project_panel->bg_video, files_field_rect);

  EndScissorMode();
  GuiWindowFileDialog(&project_panel->audio.state);
  GuiWindowFileDialog(&project_panel->jacket.state);
  GuiWindowFileDialog(&project_panel->background.state);
  GuiWindowFileDialog(&project_panel->bg_video.state);
  BeginScissorMode(project_panel->scroll_rect.x, project_panel->scroll_rect.y, project_panel->scroll_rect.width, project_panel->scroll_rect.height);

  EndScissorMode();
  EndShaderMode();
}

void project_panel_add_missing_codepoints(ProjectPanel *project_panel, List *codepoints, List *font_fallbacks)
{
  if (project_panel->is_text_edited)
  {
    font_add_missing_copepoints(codepoints, project_panel->title.text      , font_fallbacks);
    font_add_missing_copepoints(codepoints, project_panel->composer.text   , font_fallbacks);
    font_add_missing_copepoints(codepoints, project_panel->illustrator.text, font_fallbacks);
    font_add_missing_copepoints(codepoints, project_panel->charter.text    , font_fallbacks);
    font_add_missing_copepoints(codepoints, project_panel->diff_text.text  , font_fallbacks);
    font_add_missing_copepoints(codepoints, project_panel->alias.text      , font_fallbacks);
    font_add_missing_copepoints(codepoints, project_panel->bpm_text.text   , font_fallbacks);
    font_add_missing_copepoints(codepoints, project_panel->search_tags.text, font_fallbacks);
  }
}

void project_panel_add_str_to_codepoints(ProjectPanel *project_panel, List *codepoints)
{
  if (project_panel->is_text_edited)
  {
    font_add_string_to_codepoints(codepoints, project_panel->title.text);
    font_add_string_to_codepoints(codepoints, project_panel->composer.text);
    font_add_string_to_codepoints(codepoints, project_panel->illustrator.text);
    font_add_string_to_codepoints(codepoints, project_panel->charter.text);
    font_add_string_to_codepoints(codepoints, project_panel->diff_text.text);
    font_add_string_to_codepoints(codepoints, project_panel->alias.text);
    font_add_string_to_codepoints(codepoints, project_panel->bpm_text.text);
    font_add_string_to_codepoints(codepoints, project_panel->search_tags.text);
  }
}

void project_panel_apply_chart(ProjectPanel *project_panel, ChartSettings *chart_settings)
{
  project_panel->title = rgui_textinput_init(chart_settings->title);
  project_panel->composer = rgui_textinput_init(chart_settings->composer);
  project_panel->illustrator = rgui_textinput_init(chart_settings->illustrator);
  project_panel->charter = rgui_textinput_init(chart_settings->charter);
  project_panel->diff_text = rgui_textinput_init(chart_settings->difficulty);
  project_panel->alias = rgui_textinput_init(chart_settings->alias);

  snprintf(project_panel->base_bpm.text, sizeof(project_panel->base_bpm.text), "%.*f", PROJ_BASE_BPM_DECIMALS, chart_settings->base_bpm);
  project_panel->base_bpm = rgui_floatinput_init(project_panel->base_bpm.text);
  project_panel->is_sync = chart_settings->sync_base_bpm;
  project_panel->bpm_text = rgui_textinput_init(chart_settings->bpm_text);
  snprintf(project_panel->chart_offset.text, sizeof(project_panel->chart_offset.text), "%d", chart_settings->audio_offset);
  project_panel->chart_offset = rgui_intinput_init(project_panel->chart_offset.text);
  project_panel->judge_density = rgui_intinput_init("0");
  snprintf(project_panel->cc.text, sizeof(project_panel->cc.text), "%.*f", PROJ_FLOAT_DECIMALS, chart_settings->chart_constant);
  project_panel->cc = rgui_floatinput_init(project_panel->cc.text);
  snprintf(project_panel->preview_from.text, sizeof(project_panel->preview_from.text), "%d", chart_settings->preview_from);
  project_panel->preview_from = rgui_intinput_init(project_panel->preview_from.text);
  snprintf(project_panel->preview_to.text, sizeof(project_panel->preview_to.text), "%d", chart_settings->preview_to);
  project_panel->preview_to = rgui_intinput_init(project_panel->preview_to.text);
  project_panel->search_tags = rgui_textinput_init(chart_settings->search_tags);

  text_copy_bounded(project_panel->audio.file, sizeof(project_panel->audio.file), chart_settings->audio_path);
  text_copy_bounded(project_panel->jacket.file, sizeof(project_panel->jacket.file), chart_settings->jacket_path);
  text_copy_bounded(project_panel->background.file, sizeof(project_panel->background.file), chart_settings->background_path);

  project_panel->is_text_edited = true;
}

void project_panel_unload(ProjectPanel *project_panel)
{
  rgui_fileinput_unload(&project_panel->audio);
  rgui_fileinput_unload(&project_panel->jacket);
  rgui_fileinput_unload(&project_panel->background);
  rgui_fileinput_unload(&project_panel->bg_video);
}
