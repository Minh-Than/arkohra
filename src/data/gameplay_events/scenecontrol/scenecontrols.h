#ifndef SCENECONTROLS_H
#define SCENECONTROLS_H

typedef enum {
  SC_HIDEGROUP,
  SC_GROUPALPHA,
  SC_ENWIDENCAMERA,
  SC_OTHERS,
  SC_NONE
} SCType;

SCType scenecontrol_determine_type(char *str_name);

#endif // SCENECONTROLS_H
