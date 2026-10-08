#ifndef SELECTION_SERVICE_H
#define SELECTION_SERVICE_H

#include <stdbool.h>
#include "data/custom_types/dynamic_list.h"
#include "render/track/track_service.h"
#include "windows/window_services.h"
#define MAX_RAYCAST_RESULTS 32

typedef struct {
  List selected_notes;
  List raycast_results;
} SelectionService;

SelectionService selection_service_init();
void selection_service_handle_inputs(SelectionService *selection_service, TrackService *track_service, WindowGroup *window_services, bool is_chart_initiailzed);
void selection_service_unload(SelectionService *selection_service);

#endif // SELECTION_SERVICE_H
