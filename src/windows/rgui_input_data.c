#include <float.h>
#include "raylib.h"
#include "raygui.h"
#include "rgui_input_data.h"
#include "data/app_configs/app_config.h"
#include "data/fonts/fonts_service.h"

#ifdef _WIN32
#define PATH_SEPERATOR "\\"
#else
#define PATH_SEPERATOR "/"
#endif

RguiTextInput rgui_textinput_init(const char* value)
{
  RguiTextInput input = { 0 };
  TextCopy(input.text, value);
  TextCopy(input.snapshot, value);

  return input;
}

RguiIntInput rgui_intinput_init(const char* value)
{
  RguiIntInput input = { 0 };
  TextCopy(input.text, value);

  return input;
}

int rgui_intinput_textbox(RguiIntInput *input, Rectangle rect)
{
  int result = GuiTextBox(rect, input->text, sizeof(input->text), input->edit);
  if (result)
  {
    input->edit = !input->edit;
    input->value = text_to_int_validated(input->text, input->value, INT_MIN, INT_MAX);
    snprintf(input->text, sizeof(input->text), "%d", input->value);
  } else
  {
    input->value = text_to_int_validated(input->text, input->value, INT_MIN, INT_MAX);
  }

  return result;
}

RguiFloatInput rgui_floatinput_init(const char* value)
{
  RguiFloatInput input = { 0 };
  TextCopy(input.text, value);

  return input;
}

int rgui_floatinput_textbox(RguiFloatInput *input, Rectangle rect, int decimal)
{
  int result = GuiTextBox(rect, input->text, sizeof(input->text), input->edit);
  if (result)
  {
    input->edit = !input->edit;
    input->value = text_to_float_validated(input->text, input->value, -FLT_MAX, FLT_MAX);
    snprintf(input->text, sizeof(input->text), "%.*f", decimal, input->value);
  } else
    input->value = text_to_float_validated(input->text, input->value, -FLT_MAX, FLT_MAX);

  return result;
}

RguiFileInput rgui_fileinput_init()
{
  RguiFileInput input = { 0 };
  TextCopy(input.file, "");
  List extensions; list_init(&extensions, sizeof(char *));
  input.extensions = extensions;
  input.state = InitGuiWindowFileDialog(NULL);

  return input;
}

int rgui_fileinput_button(RguiFileInput *input, Rectangle rect)
{
  int result = GuiButton(rect, GetFileName(input->file));

  if(result) input->state.windowActive = true;

  if(input->state.SelectFilePressed)
  {
    bool is_valid = false;
    for (int i = 0; i < input->extensions.size; i++)
    {
      const char* ext = (char *)list_get(&input->extensions, i);
      is_valid = IsFileExtension(input->state.fileNameText, ext);
      if (is_valid) break;
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
