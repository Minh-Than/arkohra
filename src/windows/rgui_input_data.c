#include "raylib.h"
#include "rgui_input_data.h"

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
