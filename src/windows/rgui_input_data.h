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

int rgui_textinput_draw_font(Rectangle bounds, char *text, int textSize, float fontSize, float spacing, bool editMode, List *fontList);
RguiTextInput rgui_textinput_init(const char* value);

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
int rgui_intinput_textbox(RguiIntInput *input, Rectangle rect);

typedef struct {
  char text[RGUI_INPUT_TEXT_CAP];
  bool edit;
  float value;
} RguiFloatInput;

RguiFloatInput rgui_floatinput_init(const char* value);
int rgui_floatinput_textbox(RguiFloatInput *input, Rectangle rect, int decimal);

typedef struct {
  List extensions;
  char file[MAXPATHLEN];
  GuiWindowFileDialogState state;
} RguiFileInput;

RguiFileInput rgui_fileinput_init();
int rgui_fileinput_button(RguiFileInput *input, Rectangle rect);
void rgui_fileinput_unload(RguiFileInput *input);

typedef struct {
  char list[MAXPATHLEN];
  bool edit;
  int option;
} RguiSelectInput;

RguiSelectInput rgui_selectinput_init(const char* list, int option);
int rgui_selectinput_dropdown(RguiSelectInput *input, Rectangle rect);

#endif // RGUI_INPUT_DATA_H
