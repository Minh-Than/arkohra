#ifndef RAYCAST_NOTE_H
#define RAYCAST_NOTE_H

#include <stdbool.h>
#include "data/gameplay_events/gameplay_events.h"
#include "raylib.h"

typedef struct {
  void *data;
  RayCollision collision;
  RawEventType note_type;
}RaycastNote;

int raycast_note_compare_distance_asc(const void *a, const void *b);
int raycast_note_compare_data(const void *a, const void *b);

#endif // RAYCAST_NOTE_H
