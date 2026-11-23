#include "raylib.h"
#include "libraries/raygui.h"
#include "constants.h"
#include "rgui_input_data.h"
#include "data/app_configs/app_config.h"
#include "data/custom_types/custom_types.h"
#include "raymath.h"

RguiTextInput rgui_textinput_init(const char* value)
{
  RguiTextInput input = { 0 };
  TextCopy(input.text, value);
  TextCopy(input.snapshot, value);

  return input;
}

RguiIntInput rgui_intinput_init(int value)
{
  RguiIntInput input = { 0 };
  input.value = value;
  snprintf(input.text, sizeof(input.text), "%d", value);

  return input;
}

static void text_to_int_commit(RguiIntInput *input, int min, int max)
{
  input->value = text_to_int_validated(input->text, input->value, min, max);
  snprintf(input->text, sizeof(input->text), "%d", input->value);
}

int rgui_intinput_textbox(RguiIntInput *input, Rectangle rect, int min, int max)
{
  int result = GuiTextBox(rect, input->text, sizeof(input->text), input->edit);

  if (!result) return 0;
  if (input->edit) text_to_int_commit(input, min, max);
  input->edit = !input->edit;
  return result;
}

RguiFloatInput rgui_floatinput_init(float value, int decimals)
{
  RguiFloatInput input = { 0 };
  input.value = value;
  snprintf(input.text, sizeof(input.text), "%.*f", decimals, value);

  return input;
}

static void text_to_float_commit(RguiFloatInput *input, int decimal, int min, int max)
{
  input->value = text_to_float_validated(input->text, input->value, min, max);
  snprintf(input->text, sizeof(input->text), "%.*f", decimal, input->value);
}

int rgui_floatinput_textbox(RguiFloatInput *input, Rectangle rect, int decimal, float min, float max)
{
  int result = GuiTextBox(rect, input->text, sizeof(input->text), input->edit);

  if (!result) return 0;
  if (input->edit) text_to_float_commit(input, decimal, min, max);
  input->edit = !input->edit;
  return result;
}

int rgui_floatinput_spinner(RguiFloatInput *input, Rectangle rect, int decimal, float min, float max, float increment)
{
  const float button_width = 24.0f;
  const float gap = 4.0f;

  Rectangle decrement_button_rect = { rect.x,                             rect.y, button_width, rect.height };
  Rectangle increment_button_rect = { rect.x + rect.width - button_width, rect.y, button_width, rect.height };
  Rectangle field_rect = { rect.x + button_width + gap, rect.y, rect.width - (button_width + gap)*2, rect.height };

  int decrement_result = GuiButton(decrement_button_rect, GuiIconText(ICON_ARROW_LEFT_FILL , NULL));
  int increment_result = GuiButton(increment_button_rect, GuiIconText(ICON_ARROW_RIGHT_FILL, NULL));
  int field_result = GuiTextBox(field_rect, input->text, sizeof(input->text), input->edit);

  if (decrement_result ^ increment_result)
  {
    float sign = decrement_result ? -1.0f : 1.0f;
    input->value = Clamp(input->value + increment*sign, min, max);
    snprintf(input->text, sizeof(input->text), "%.*f", decimal, input->value);
    return 1;
  }

  if (!field_result) return 0;
  if (input->edit) text_to_float_commit(input, decimal, min, max);
  input->edit = !input->edit;
  return field_result;
}

RguiFileInput rgui_fileinput_init(const char* path)
{
  RguiFileInput input = { 0 };
  text_copy_bounded(input.file, sizeof(input.file), path);
  List extensions; list_init(&extensions, sizeof(char *));
  input.extensions = extensions;
  input.state = InitGuiWindowFileDialog(NULL);

  return input;
}

int rgui_fileinput_button(RguiFileInput *input, Rectangle rect)
{
  int result = GuiButton(rect, input->file[0] != '\0'
                               ? GetFileName(input->file)
                               : GuiIconText(ICON_FILE_OPEN, "Upload a file . . ."));
  if(!result) return 0;

  input->state.windowActive = true;
  if(input->state.SelectFilePressed)
  {
    bool is_valid = false;
    if (input->extensions.size == 0) is_valid = true;
    else
    {
      for (int i = 0; i < input->extensions.size; i++)
      {
        const char* ext = (char *)list_get(&input->extensions, i);
        is_valid = IsFileExtension(input->state.fileNameText, ext);
        if (is_valid) break;
      }
    }
    if(is_valid)
      strcpy(input->file, TextFormat("%s%s%s", input->state.dirPathText, PATH_SEPERATOR, input->state.fileNameText));
    input->state.SelectFilePressed = false;
  }

  return result;
}

void rgui_fileinput_unload(RguiFileInput *input)
{
  list_free(&input->extensions);
}

// list: "ITEM_1;ITEM_2;ITEM_3"
RguiSelectInput rgui_selectinput_init(const char* list, int option)
{
  RguiSelectInput input = { 0 };
  text_copy_bounded(input.list, MAXPATHLEN, list);
  input.option = option;

  return input;
}

int rgui_selectinput_dropdown(RguiSelectInput *input, Rectangle rect)
{
  int result = GuiDropdownBox(rect, input->list, &input->option, input->edit);
  if (result) input->edit = !input->edit;

  return result;
}

int rgui_selectinput_toggles(RguiSelectInput *input, Rectangle rect)
{
  int result = GuiToggleGroup(rect, input->list, &input->option);

  return result;
}

RguiRawFloatInput rgui_rawfloatinput_init(float value)
{
  RguiRawFloatInput input = { 0 };
  input.value = value;

  return input;
}
