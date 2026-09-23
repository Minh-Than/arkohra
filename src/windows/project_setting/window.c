#include <float.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data/custom_types/dynamic_list.h"
#include "gameplay/camera/camera_service.h"
#include "raylib.h"
#include "raygui.h"
#include "raymath.h"
#include "data/fonts/fonts_service.h"
#include "windows/window_inst.h"
#include "windows/rgui_input_data.h"
#include "../project_setting/window.h"
#include "gui_window_file_dialog.h"

#ifdef _WIN32
#define PATH_SEPERATOR "\\"
#else
#define PATH_SEPERATOR "/"
#endif

ProjSettingData proj_setting_init(int glsl, AppConfigs *app_configs)
{
  ProjSettingData data = { 0 };
  data.proj_scroll = data.general_scroll = Vector2Zero();
  data.scroll_rect = (Rectangle){ 0, 0 };
  data.setting_options_active = 2;

  data.title = data.composer = data.illustrator = data.charter = data.diff_text = data.alias  = rgui_textinput_init("");

  data.bpm_text = data.search_tag = rgui_textinput_init("");
  data.base_bpm = data.cc = rgui_floatinput_init("0.0");
  data.chart_offset = data.judge_density = data.preview_from = rgui_intinput_init("0");
  data.preview_to = rgui_intinput_init("10000");

  data.audio = data.jacket = data.background = data.bg_video = rgui_fileinput_init("");

  snprintf(data.scroll_speed.text, sizeof(data.scroll_speed.text), "%.2f", app_configs->scroll_speed);
  data.scroll_speed = rgui_floatinput_init(data.scroll_speed.text);
  data.aspect_ratio = rgui_selectinput_init("16 : 9;20 : 9;18 : 9;4 : 3;3 : 2", app_configs->playfield_ratio);
    printf("%s\n", data.aspect_ratio.list);

  snprintf(data.music_volume.text, sizeof(data.music_volume.text), "%.2f", app_configs->music_volume);
  data.music_volume = rgui_floatinput_init(data.music_volume.text);
  snprintf(data.effect_volume.text, sizeof(data.effect_volume.text), "%.2f", app_configs->hit_volume);
  data.effect_volume = rgui_floatinput_init(data.effect_volume.text);

  data.sdf_shader = LoadShader(0, TextFormat("resources/shaders/glsl%i/sdf.fs", glsl));
  const char *paths[] = { "resources/fonts/NotoSans-Regular.ttf", };
  List font_list; list_init(&font_list, sizeof(char *));
  for (int i = 0; i < 1; i++) list_push(&font_list, &paths[i]);
  List hud_code_points; list_init(&hud_code_points, sizeof(int));
  for (int cp = 0x20; cp <= 0x7E; cp++) list_push(&hud_code_points, &cp);
  data.font_fallbacks = fonts_init(&font_list, 45, (int *)hud_code_points.data, hud_code_points.size); 
  list_free(&hud_code_points);
  list_free(&font_list);

  return data;
}

void proj_setting_draw(WindowInst* window_inst, ProjSettingData *data, AppConfigs *app_configs, RenderContext *render_ctx)
{
  Rectangle win_rect = (Rectangle){ window_inst->x, window_inst->y,
                                    window_inst->width, window_inst->height };

  static bool resizing = false;
  Rectangle handle  = (Rectangle){ win_rect.x + win_rect.width  - 10,
                                   win_rect.y + win_rect.height - 10,
                                   20, 20 };
  if (resizing)
  {
    window_inst->width  = fmaxf(400, fminf(GetScreenWidth() , GetMouseX() - window_inst->x));
    window_inst->height = fmaxf(400, fminf(GetScreenHeight(), GetMouseY() - window_inst->y));
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) resizing = false;
  }
  else if (CheckCollisionPointRec(GetMousePosition(), handle) &&
           IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
  {
    resizing = true;
  }

  static bool dragging = false;
  static Vector2 dragging_pos = { 0, 0 };
  Rectangle top_bar = (Rectangle){ win_rect.x, win_rect.y, win_rect.width, 24 };
  if(dragging)
  {
    window_inst->x = GetMouseX() - dragging_pos.x;
    window_inst->y = GetMouseY() - dragging_pos.y;
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) dragging = false;
  }
  else if (CheckCollisionPointRec(GetMousePosition(), top_bar) &&
           IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
  {
    dragging = true;
    dragging_pos.x = GetMouseX() - window_inst->x;
    dragging_pos.y = GetMouseY() - window_inst->y;
  }

  data->setting_window_active = !GuiWindowBox(win_rect, "PROJECT SETTINGS");
  if (!data->setting_window_active) window_inst_close(window_inst);

  // --- layout, relative to the window ---
  int content_x = window_inst->x + 8;
  int toggle_y  = (int)window_inst->y + 32;
  int scroll_y  = toggle_y + 32;
  int scroll_w  = (int)window_inst->width  - 16;
  int scroll_h  = (int)window_inst->height - (scroll_y - (int)window_inst->y) - 8;

  GuiToggleGroup((Rectangle){content_x, toggle_y, 56, 24},
                 "Project;Events;Settings", &data->setting_options_active);

  DrawTriangle((Vector2){ handle.x + 8, handle.y + 8 },
               (Vector2){ handle.x + 8, handle.y - 1 },
               (Vector2){ handle.x - 2, handle.y + 8 }, GRAY);

  // PROJECT
  if (data->setting_options_active == 0)
  {
    BeginShaderMode(data->sdf_shader);

    int info_panel_h = 200;
    int gameplay_panel_h = 230;
    int files_panel_h = 140;

    int input_field_h = 24;
    int input_field_gap = 30;
    int total_content_h = info_panel_h + gameplay_panel_h + files_panel_h + 12*3 + 8;
    GuiScrollPanel((Rectangle){content_x, scroll_y, scroll_w, scroll_h},
                   NULL, (Rectangle){0, 0, scroll_w, total_content_h},
                   &data->proj_scroll, &data->scroll_rect);

    BeginScissorMode(data->scroll_rect.x, data->scroll_rect.y, data->scroll_rect.width, data->scroll_rect.height);

    // Information
    int info_panel_x = content_x + 8    + (int)data->proj_scroll.x;
    int info_panel_y = scroll_y  + 12*1 + (int)data->proj_scroll.y;
    int info_panel_w = scroll_w  - 28;
    int label_xs = 96;
    GuiGroupBox ((Rectangle){info_panel_x, info_panel_y, info_panel_w, info_panel_h}, "[ INFORMATION ]");

    int info_field_x = info_panel_x + 8;
    int info_field_y = info_panel_y + 16;

    char *info_labels[] = { "Title", "Composer", "Illustrator", "Charter", "Difficulty Name", "Alias" };
    char *info_values[] = {
      data->title.text, data->composer.text,
      data->illustrator.text, data->charter.text,
      data->diff_text.text, data->alias.text
    };
    char *info_snapshots[] = {
      data->title.snapshot, data->composer.snapshot,
      data->illustrator.snapshot, data->charter.snapshot,
      data->diff_text.snapshot, data->alias.snapshot
    };
    bool *info_edit[] = {
      &data->title.edit, &data->composer.edit,
      &data->illustrator.edit, &data->charter.edit,
      &data->diff_text.edit, &data->alias.edit
    };
    for (int i = 0; i < 6; i++)
    {
      int row_y = info_field_y + input_field_gap * i;
      GuiLabel((Rectangle){ info_field_x, row_y, label_xs, input_field_h }, info_labels[i]);

      bool wasEditing = *info_edit[i];
      if (rgui_textinput_draw_font((Rectangle){ info_field_x + label_xs, row_y, info_panel_w - 16 - label_xs, input_field_h },
                          info_values[i], RGUI_INPUT_TEXT_CAP, 20, 0.6f, wasEditing, &data->font_fallbacks))
      {
        *info_edit[i] = !wasEditing;
        if (!wasEditing) snprintf(info_snapshots[i], RGUI_INPUT_TEXT_CAP, "%s", info_values[i]);
        else if (strcmp(info_snapshots[i], info_values[i]) != 0) proj_setting_reload_font(data);
      }
      // rgui_textinput_draw_font() ends scissor mode
      BeginScissorMode(data->scroll_rect.x, data->scroll_rect.y, data->scroll_rect.width, data->scroll_rect.height);
    }

    // Gameplay
    int gameplay_panel_x = content_x + 8    + (int)data->proj_scroll.x;
    int gameplay_panel_y = scroll_y  + 12*2 + (int)data->proj_scroll.y + info_panel_h;
    int gameplay_panel_w = scroll_w  - 28;
    GuiGroupBox ((Rectangle){gameplay_panel_x, gameplay_panel_y, gameplay_panel_w, gameplay_panel_h}, "[ GAMEPLAY ]");

    int gameplay_field_x = gameplay_panel_x + 8;
    int gameplay_field_y = gameplay_panel_y + 16;

    int is_sync_sub_x = 70;
    int gameplay_input_x = info_field_x + label_xs;
    int base_bpm_checkbox_x = gameplay_input_x + info_panel_w - 16 - label_xs - is_sync_sub_x + 8;
    GuiLabel((Rectangle){ gameplay_field_x, gameplay_field_y + input_field_gap*0,
                          label_xs, input_field_h },
             "Base BPM");
    if (GuiTextBox((Rectangle){ gameplay_input_x, gameplay_field_y + input_field_gap*0,
                                info_panel_w - 16 - label_xs - is_sync_sub_x, input_field_h },
                   data->base_bpm.text, RGUI_INPUT_TEXT_CAP, data->base_bpm.edit))
    {
      data->base_bpm.edit = !data->base_bpm.edit;
      data->base_bpm.value = text_to_float_validated(data->base_bpm.text, data->base_bpm.value, -FLT_MAX, FLT_MAX);

      int dec = fminf(12, count_decimals(data->base_bpm.text));
      snprintf(data->base_bpm.text, sizeof(data->base_bpm.text), "%.*f", dec, data->base_bpm.value);
    } else
    {
      data->cc.value = text_to_float_validated(data->cc.text, data->cc.value, -FLT_MAX, FLT_MAX);
    }
    GuiCheckBox((Rectangle){base_bpm_checkbox_x, gameplay_field_y + input_field_gap*0, input_field_h, input_field_h},
                "Sync", &data->is_sync);
    GuiLabel((Rectangle){ gameplay_field_x, gameplay_field_y + input_field_gap*1,
                          label_xs, input_field_h },
             "BPM Text");
    if (rgui_textinput_draw_font((Rectangle){ gameplay_input_x, gameplay_field_y + input_field_gap*1,
                                info_panel_w - 16 - label_xs, input_field_h },
                   data->bpm_text.text, RGUI_INPUT_TEXT_CAP, 20, 1.0f, data->bpm_text.edit, &data->font_fallbacks))
    {
      data->bpm_text.edit = !data->bpm_text.edit;
    }
    GuiLabel((Rectangle){ gameplay_field_x, gameplay_field_y + input_field_gap*2,
                          label_xs, input_field_h },
             "Chart Offset");
    if (GuiTextBox((Rectangle){ gameplay_input_x, gameplay_field_y + input_field_gap*2,
                                 info_panel_w - 16 - label_xs, input_field_h },
                    data->chart_offset.text, RGUI_INPUT_TEXT_CAP, data->chart_offset.edit))
    {
      data->chart_offset.edit = !data->chart_offset.edit;
      data->chart_offset.value = text_to_int_validated(data->chart_offset.text, data->chart_offset.value, INT_MIN, INT_MAX);
      snprintf(data->chart_offset.text, sizeof(data->chart_offset.text), "%d", data->chart_offset.value);
    } else
    {
      data->chart_offset.value = text_to_int_validated(data->chart_offset.text, data->chart_offset.value, INT_MIN, INT_MAX);
    }
    GuiLabel((Rectangle){ gameplay_field_x, gameplay_field_y + input_field_gap*3,
                          label_xs, input_field_h },
             "Judge Density");
    if (GuiTextBox((Rectangle){ gameplay_input_x, gameplay_field_y + input_field_gap*3,
                                 info_panel_w - 16 - label_xs, input_field_h },
                    data->judge_density.text, RGUI_INPUT_TEXT_CAP, data->judge_density.edit))
    {
      data->judge_density.edit = !data->judge_density.edit;
      data->judge_density.value = text_to_int_validated(data->judge_density.text, data->judge_density.value, INT_MIN, INT_MAX);
      snprintf(data->judge_density.text, sizeof(data->judge_density.text), "%d", data->judge_density.value);
    } else
    {
      data->judge_density.value = text_to_int_validated(data->judge_density.text, data->judge_density.value, INT_MIN, INT_MAX);
    }
    GuiLabel((Rectangle){ gameplay_field_x, gameplay_field_y + input_field_gap*4,
                          label_xs, input_field_h },
             "Chart Constant");
    if (GuiTextBox((Rectangle){ gameplay_input_x, gameplay_field_y + input_field_gap*4,
                                info_panel_w - 16 - label_xs, input_field_h },
                   data->cc.text, RGUI_INPUT_TEXT_CAP, data->cc.edit))
    {
      data->cc.edit = !data->cc.edit;
      data->cc.value = text_to_float_validated(data->cc.text, data->cc.value, -FLT_MAX, FLT_MAX);

      int dec = fminf(12, count_decimals(data->cc.text));
      snprintf(data->cc.text, sizeof(data->cc.text), "%.*f", dec, data->cc.value);
    } else
    {
      data->cc.value = text_to_float_validated(data->cc.text, data->cc.value, -FLT_MAX, FLT_MAX);
    }
    GuiLabel((Rectangle){ gameplay_field_x, gameplay_field_y + input_field_gap*5,
                          label_xs, input_field_h },
             "Preview Segment");
    if (GuiTextBox((Rectangle){ gameplay_input_x, gameplay_field_y + input_field_gap*5,
                                 ((float)info_panel_w / 2) - label_xs + 38, input_field_h },
                    data->preview_from.text, 256, data->preview_from.edit))
    {
      data->preview_from.edit = !data->preview_from.edit;
      data->preview_from.value = text_to_int_validated(data->preview_from.text, data->preview_from.value, INT_MIN, INT_MAX);
      snprintf(data->preview_from.text, sizeof(data->preview_from.text), "%d", data->preview_from.value);
    } else
    {
      data->preview_from.value = text_to_int_validated(data->preview_from.text, data->preview_from.value, INT_MIN, INT_MAX);
    }
    if (GuiTextBox((Rectangle){ gameplay_input_x + ((float)info_panel_w / 2) - label_xs + 44, gameplay_field_y + input_field_gap*5,
                                 ((float)info_panel_w / 2) - label_xs + 36, input_field_h },
                    data->preview_to.text, RGUI_INPUT_TEXT_CAP, data->preview_to.edit))
    {
      data->preview_to.edit = !data->preview_to.edit;
      data->preview_to.value = text_to_int_validated(data->preview_to.text, data->preview_to.value, INT_MIN, INT_MAX);
      snprintf(data->preview_to.text, sizeof(data->preview_to.text), "%d", data->preview_to.value);
    } else
    {
      data->preview_to.value = text_to_int_validated(data->preview_to.text, data->preview_to.value, INT_MIN, INT_MAX);
    }
    GuiLabel((Rectangle){ gameplay_field_x, gameplay_field_y + input_field_gap*6,
                          label_xs, input_field_h },
             "Search Tag");
    if (rgui_textinput_draw_font((Rectangle){ gameplay_input_x, gameplay_field_y + input_field_gap*6,
                                info_panel_w - 16 - label_xs, input_field_h },
                   data->search_tag.text, RGUI_INPUT_TEXT_CAP, 20, 0.6f, data->search_tag.edit, &data->font_fallbacks))
    {
      data->search_tag.edit = !data->search_tag.edit;
    }

    // Files
    int files_panel_x = content_x + 8    + (int)data->proj_scroll.x;
    int files_panel_y = scroll_y  + 12*3 + (int)data->proj_scroll.y + info_panel_h + gameplay_panel_h;
    int files_panel_w = scroll_w  - 28;
    GuiGroupBox ((Rectangle){files_panel_x, files_panel_y, files_panel_w, files_panel_h}, "[ FILES ]");

    int files_field_x = files_panel_x + 8;
    int files_field_y = files_panel_y + 16;

    int files_input_x = info_field_x + label_xs;

    GuiLabel((Rectangle){ files_field_x, files_field_y + input_field_gap*0,
                          label_xs, input_field_h },
             "Audio");
    if(GuiButton((Rectangle){ files_input_x, files_field_y + input_field_gap*0,
                              info_panel_w - label_xs - 16, input_field_h },
                 GetFileName(data->audio.file)))
    {
      data->audio.state.windowActive = true;
    }
    if(data->audio.state.SelectFilePressed)
    {
      if(IsFileExtension(data->audio.state.fileNameText, ".ogg"))
        strcpy(data->audio.file, TextFormat("%s%s%s", data->audio.state.dirPathText, PATH_SEPERATOR, data->audio.state.fileNameText));
      data->audio.state.SelectFilePressed = false;
    }
    GuiLabel((Rectangle){ files_field_x, files_field_y + input_field_gap*1,
                          label_xs, input_field_h },
             "Jacket Art");
    if(GuiButton((Rectangle){ files_input_x, files_field_y + input_field_gap*1,
                              info_panel_w - label_xs - 16, input_field_h },
                 GetFileName(data->jacket.file)))
    {
      data->jacket.state.windowActive = true;
    }
    if(data->jacket.state.SelectFilePressed)
    {
      if(IsFileExtension(data->jacket.state.fileNameText, ".png") ||
         IsFileExtension(data->jacket.state.fileNameText, ".jpg") ||
         IsFileExtension(data->jacket.state.fileNameText, ".jpeg"))
        strcpy(data->jacket.file, TextFormat("%s%s%s", data->jacket.state.dirPathText, PATH_SEPERATOR, data->jacket.state.fileNameText));
      data->jacket.state.SelectFilePressed = false;
    }
    GuiLabel((Rectangle){ files_field_x, files_field_y + input_field_gap*2,
                          label_xs, input_field_h },
             "Background");
    if(GuiButton((Rectangle){ files_input_x, files_field_y + input_field_gap*2,
                              info_panel_w - label_xs - 16, input_field_h },
                 GetFileName(data->background.file)))
    {
      data->background.state.windowActive = true;
    }
    if(data->background.state.SelectFilePressed)
    {
      if(IsFileExtension(data->background.state.fileNameText, ".png") ||
         IsFileExtension(data->background.state.fileNameText, ".jpg") ||
         IsFileExtension(data->background.state.fileNameText, ".jpeg"))
        strcpy(data->background.file, TextFormat("%s%s%s", data->background.state.dirPathText, PATH_SEPERATOR, data->background.state.fileNameText));
      data->background.state.SelectFilePressed = false;
    }
    GuiLabel((Rectangle){ files_field_x, files_field_y + input_field_gap*3,
                          label_xs, input_field_h },
             "Video");
    if(GuiButton((Rectangle){ files_input_x, files_field_y + input_field_gap*3,
                              info_panel_w - label_xs - 16, input_field_h },
                 GetFileName(data->bg_video.file)))
    {
      data->bg_video.state.windowActive = true;
    }
    if(data->bg_video.state.SelectFilePressed)
    {
      if(IsFileExtension(data->bg_video.state.fileNameText, ".mp4"))
        strcpy(data->bg_video.file, TextFormat("%s%s%s", data->bg_video.state.dirPathText, PATH_SEPERATOR, data->bg_video.state.fileNameText));
      data->bg_video.state.SelectFilePressed = false;
    }

    EndScissorMode();
    GuiWindowFileDialog(&data->audio.state);
    GuiWindowFileDialog(&data->jacket.state);
    GuiWindowFileDialog(&data->background.state);
    GuiWindowFileDialog(&data->bg_video.state);
    BeginScissorMode(data->scroll_rect.x, data->scroll_rect.y, data->scroll_rect.width, data->scroll_rect.height);

    EndScissorMode();
    EndShaderMode();
  }

  // GENERAL
  if (data->setting_options_active == 2)
  {
    BeginShaderMode(data->sdf_shader);

    int gameplay_panel_h = 80;
    int audio_panel_h = 80;

    int input_field_h = 24;
    int input_field_gap = 30;
    int total_content_h = gameplay_panel_h +audio_panel_h + 12*2 + 8;
    GuiScrollPanel((Rectangle){content_x, scroll_y, scroll_w, scroll_h},
                   NULL, (Rectangle){0, 0, scroll_w, total_content_h},
                   &data->general_scroll, &data->scroll_rect);

    int label_xs = 96;
    BeginScissorMode(data->scroll_rect.x, data->scroll_rect.y, data->scroll_rect.width, data->scroll_rect.height);

    // Audio
    int audio_panel_x = content_x + 8    + (int)data->general_scroll.x;
    int audio_panel_y = scroll_y  + 12*2 + (int)data->general_scroll.y + gameplay_panel_h;
    int audio_panel_w = scroll_w  - 28;
    GuiGroupBox ((Rectangle){audio_panel_x, audio_panel_y, audio_panel_w, audio_panel_h}, "[ AUDIO ]");

    int audio_field_x = audio_panel_x + 8;
    int audio_field_y = audio_panel_y + 16;

    int audio_input_x = audio_field_x + label_xs;

    GuiLabel((Rectangle){ audio_field_x, audio_field_y + input_field_gap*0,
                          label_xs, input_field_h },
             "Music Volume");
    if (GuiTextBox((Rectangle){ audio_input_x, audio_field_y + input_field_gap*0,
                                audio_panel_w - 16 - label_xs, input_field_h },
                   data->music_volume.text, RGUI_INPUT_TEXT_CAP, data->music_volume.edit))
    {
      data->music_volume.edit = !data->music_volume.edit;
      data->music_volume.value = text_to_float_validated(data->music_volume.text, data->music_volume.value, -FLT_MAX, FLT_MAX);

      int dec = fminf(2, fmaxf(count_decimals(data->cc.text), 2));
      snprintf(data->music_volume.text, sizeof(data->music_volume.text), "%.*f", dec, data->music_volume.value);
    } else
    {
      // --- while editing or idle: keep the float in sync ---
      data->music_volume.value = text_to_float_validated(data->music_volume.text, data->music_volume.value, -FLT_MAX, FLT_MAX);
    }
    GuiLabel((Rectangle){ audio_field_x, audio_field_y + input_field_gap*1,
                          label_xs, input_field_h },
             "Effect Volume");
    if (GuiTextBox((Rectangle){ audio_input_x, audio_field_y + input_field_gap*1,
                                audio_panel_w - 16 - label_xs, input_field_h },
                   data->effect_volume.text, RGUI_INPUT_TEXT_CAP, data->effect_volume.edit))
    {
      data->effect_volume.edit = !data->effect_volume.edit;
      data->effect_volume.value = text_to_float_validated(data->effect_volume.text, data->effect_volume.value, -FLT_MAX, FLT_MAX);

      int dec = fminf(2, fmaxf(count_decimals(data->cc.text), 2));
      snprintf(data->effect_volume.text, sizeof(data->effect_volume.text), "%.*f", dec, data->effect_volume.value);
    } else
    {
      data->effect_volume.value = text_to_float_validated(data->effect_volume.text, data->effect_volume.value, -FLT_MAX, FLT_MAX);
    }

    // Gameplay
    // WARNING: the dropdown area are treated in rendering the same way, meaning anything drawn after WILL BE DRAWN ON TOP OF IT
    int gameplay_panel_x = content_x + 8    + (int)data->general_scroll.x;
    int gameplay_panel_y = scroll_y  + 12*1 + (int)data->general_scroll.y;
    int gameplay_panel_w = scroll_w  - 28;
    GuiGroupBox ((Rectangle){gameplay_panel_x, gameplay_panel_y, gameplay_panel_w, gameplay_panel_h}, "[ GAMEPLAY ]");

    int gameplay_field_x = gameplay_panel_x + 8;
    int gameplay_field_y = gameplay_panel_y + 16;

    int gameplay_input_x = gameplay_field_x + label_xs;

    GuiLabel((Rectangle){ gameplay_field_x, gameplay_field_y + input_field_gap*0,
                          label_xs, input_field_h },
             "Chart Speed");
    if (GuiTextBox((Rectangle){ gameplay_input_x, gameplay_field_y + input_field_gap*0,
                                gameplay_panel_w - 16 - label_xs, input_field_h },
                   data->scroll_speed.text, RGUI_INPUT_TEXT_CAP, data->scroll_speed.edit))
    {
      data->scroll_speed.edit = !data->scroll_speed.edit;
      data->scroll_speed.value = text_to_float_validated(data->scroll_speed.text, data->scroll_speed.value, -FLT_MAX, FLT_MAX);

      int dec = fminf(2, fmaxf(count_decimals(data->scroll_speed.text), 2));
      snprintf(data->scroll_speed.text, sizeof(data->scroll_speed.text), "%.*f", dec, data->scroll_speed.value);
    } else
    {
      data->scroll_speed.value = text_to_float_validated(data->scroll_speed.text, data->scroll_speed.value, -FLT_MAX, FLT_MAX);
    }
    GuiLabel((Rectangle){ gameplay_field_x, gameplay_field_y + input_field_gap*1,
                          label_xs, input_field_h },
             "Aspect Ratio");
    EndScissorMode();
    if (GuiDropdownBox((Rectangle){ gameplay_input_x, gameplay_field_y + input_field_gap*1,
                                    gameplay_panel_w - 16 - label_xs, input_field_h },
                       data->aspect_ratio.list, &data->aspect_ratio.option, data->aspect_ratio.edit))
    {
      data->aspect_ratio.edit = !data->aspect_ratio.edit;
      app_configs->playfield_ratio = data->aspect_ratio.option;
      SetWindowSize(GetScreenWidth(), (int)aspect_ratio_get_height((float)GetScreenWidth(), app_configs->playfield_ratio));
      recalibrate_camera(&render_ctx->camera);
    }
    BeginScissorMode(data->scroll_rect.x, data->scroll_rect.y, data->scroll_rect.width, data->scroll_rect.height);
    EndScissorMode();
    EndShaderMode();
  }
}

void proj_setting_unload(ProjSettingData *proj_setting)
{
  for (int i = 0; i < proj_setting->font_fallbacks.size; i++)
  {
    Font *font = (Font *)list_get(&proj_setting->font_fallbacks, i);
    UnloadFont(*font);
  }
  list_free(&proj_setting->font_fallbacks);
}

void proj_setting_reload_font(ProjSettingData *proj_setting)
{
  List missing_codepoints; list_init(&missing_codepoints, sizeof(int));
  font_add_missing_copepoints(&proj_setting->font_fallbacks, proj_setting->title.text      , &missing_codepoints);
  font_add_missing_copepoints(&proj_setting->font_fallbacks, proj_setting->composer.text   , &missing_codepoints);
  font_add_missing_copepoints(&proj_setting->font_fallbacks, proj_setting->illustrator.text, &missing_codepoints);
  font_add_missing_copepoints(&proj_setting->font_fallbacks, proj_setting->charter.text    , &missing_codepoints);
  font_add_missing_copepoints(&proj_setting->font_fallbacks, proj_setting->diff_text.text  , &missing_codepoints);
  font_add_missing_copepoints(&proj_setting->font_fallbacks, proj_setting->alias.text      , &missing_codepoints);
  font_add_missing_copepoints(&proj_setting->font_fallbacks, proj_setting->bpm_text.text   , &missing_codepoints);
  font_add_missing_copepoints(&proj_setting->font_fallbacks, proj_setting->search_tag.text , &missing_codepoints);

  if (missing_codepoints.size > 0)
  {
    const char *paths[] = {
      "resources/fonts/NotoSans-Regular.ttf",
      "resources/fonts/NotoSansSC-Regular.ttf",
      "resources/fonts/NotoSansJP-Regular.ttf",
      "resources/fonts/NotoSansKR-Regular.ttf",
      "resources/fonts/NotoSansMath-Regular.ttf",
    };
    List font_list; list_init(&font_list, sizeof(char *));
    for (int j = 0; j < 5; j++) list_push(&font_list, &paths[j]);

    // If the fallback chain gets too long, the app might lag from drawing
    // --> unload all fonts then reprocess needed codepoints
    if (proj_setting->font_fallbacks.size > 16)
    {
      List hud_code_points; list_init(&hud_code_points, sizeof(int));
      font_add_string_to_codepoints(&hud_code_points, proj_setting->title.text);
      font_add_string_to_codepoints(&hud_code_points, proj_setting->composer.text);
      font_add_string_to_codepoints(&hud_code_points, proj_setting->illustrator.text);
      font_add_string_to_codepoints(&hud_code_points, proj_setting->charter.text);
      font_add_string_to_codepoints(&hud_code_points, proj_setting->diff_text.text);
      font_add_string_to_codepoints(&hud_code_points, proj_setting->alias.text);
      font_add_string_to_codepoints(&hud_code_points, proj_setting->bpm_text.text);
      font_add_string_to_codepoints(&hud_code_points, proj_setting->search_tag.text);
      for (int cp = 0x20; cp <= 0x7E; cp++) list_push(&hud_code_points, &cp);
      for (int j = 0; j < proj_setting->font_fallbacks.size; j++)
      {
        Font *font = (Font *)list_get(&proj_setting->font_fallbacks, j);
        UnloadFont(*font);
      }
      list_free(&proj_setting->font_fallbacks);
      proj_setting->font_fallbacks = fonts_init(&font_list, 45, (int *)hud_code_points.data, hud_code_points.size);

      list_free(&hud_code_points);
    } else // Only add missing copepoints
    {
      List extra_font = fonts_init(&font_list, 45, (int *)missing_codepoints.data, missing_codepoints.size);
      for (int j = 0; j < extra_font.size; j++)
      {
        Font *font = (Font *)list_get(&extra_font, j);
        list_push(&proj_setting->font_fallbacks, font);
      }
      list_free(&extra_font);
    }
    list_free(&font_list);
  }
  list_free(&missing_codepoints);

}
