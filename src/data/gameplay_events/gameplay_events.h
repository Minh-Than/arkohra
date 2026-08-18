#ifndef GAMEPLAY_EVENTS_H
#define GAMEPLAY_EVENTS_H

#include "./tap.h"          // IWYU pragma: export
#include "./hold.h"         // IWYU pragma: export
#include "./arc.h"          // IWYU pragma: export
#include "./arctap.h"       // IWYU pragma: export
#include "./timing_event.h" // IWYU pragma: export

typedef enum
{
  TIMING_EVENT,
  TAP,
  HOLD,
  ARC,
  ARCTAP,
  TIMING_GROUP,
  NO_EVENT
} RawEventType;

RawEventType determine_type(char *line);

#endif // GAMEPLAY_EVENTS_H
