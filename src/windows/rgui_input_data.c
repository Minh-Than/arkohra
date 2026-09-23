#include "raylib.h"
#include "rgui_input_data.h"
#include "data/app_configs/app_config.h"

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

RguiFloatInput rgui_floatinput_init(const char* value)
{
  RguiFloatInput input = { 0 };
  TextCopy(input.text, value);

  return input;
}

RguiFileInput rgui_fileinput_init(const char* value)
{
  RguiFileInput input = { 0 };
  TextCopy(input.file, value);
  input.state = InitGuiWindowFileDialog(NULL);

  return input;
}

// list: "ITEM_1;ITEM_2;ITEM_3"
RguiSelectInput rgui_selectinput_init(const char* list, int option)
{
  RguiSelectInput input = { 0 };
  text_copy_bounded(input.list, MAXPATHLEN, list);
  input.option = option;

  return input;
}
