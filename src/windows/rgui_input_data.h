#ifndef RGUI_INPUT_DATA_H
#define RGUI_INPUT_DATA_H

#include <sys/param.h>
#include "data/custom_types/dynamic_list.h"
#include "gui_window_file_dialog.h"

#define RGUI_INPUT_TEXT_CAP 256

// For rgui_textinput_font() usage
typedef struct {
  char text[RGUI_INPUT_TEXT_CAP];
  char snapshot[RGUI_INPUT_TEXT_CAP];
  bool edit;
} RguiTextInput;

RguiTextInput rgui_textinput_init(const char* value);
int rgui_textinput_draw_font(Rectangle bounds, char *text, int textSize, float fontSize, float spacing, bool editMode, List *fontList);

// For GuiTextBox() usage
typedef struct {
  char text[RGUI_INPUT_TEXT_CAP];
  bool edit;
} RguiTextSimpleInput;

typedef struct {
  char text[RGUI_INPUT_TEXT_CAP];
  bool edit;
  int value;
} RguiIntInput;

RguiIntInput rgui_intinput_init(const char* value);

typedef struct {
  char text[RGUI_INPUT_TEXT_CAP];
  bool edit;
  float value;
} RguiFloatInput;

RguiFloatInput rgui_floatinput_init(const char* value);

typedef struct {
  char file[MAXPATHLEN];
  GuiWindowFileDialogState state;
} RguiFileInput;

RguiFileInput rgui_fileinput_init(const char* value);

#endif // RGUI_INPUT_DATA_H
