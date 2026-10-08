#include "selection_service.h"
#include "data/custom_types/dynamic_list.h"
#include "data/gameplay_events/gameplay_events.h"
#include "data/selection/raycast_note.h"
#include "raylib.h"
#include "windows/window_inst.h"

struct SelectedNoteHolder {
  void *data;
  RawEventType note_type;
};

SelectionService selection_service_init()
{
  List selected_notes; list_init(&selected_notes, sizeof(struct SelectedNoteHolder));
  List raycast_results; list_init(&raycast_results, sizeof(RaycastNote));

  SelectionService service = {
    .selected_notes  = selected_notes,
    .raycast_results = raycast_results
  };

  return service;
}

static bool selection_service_get_note_under_cursor(SelectionService *selection_service, RaycastNote *result)
{
  int length = selection_service->raycast_results.size;
  if (length > MAX_RAYCAST_RESULTS) length = MAX_RAYCAST_RESULTS;

  int idx_offset = 0;
  for (int i = 0; i < length; i++)
  {
    RaycastNote *ray_note = (RaycastNote *)list_get(&selection_service->raycast_results, i);
    if (!ray_note->collision.hit) continue;

    for (int j = 0; j < selection_service->selected_notes.size; j++)
    {
      struct SelectedNoteHolder *selected_note = (struct SelectedNoteHolder *)list_get(&selection_service->selected_notes, j);
      if (selected_note->data == ray_note->data) { idx_offset = i + 1; break; }
    }
  }

  for (int i = 0; i < length; i++)
  {
    RaycastNote *ray_note = (RaycastNote *)list_get(&selection_service->raycast_results, (i + idx_offset) % length);
    if (!ray_note->collision.hit) continue;

    result->data      = ray_note->data;
    result->note_type = ray_note->note_type;
    return true; // For now return true on first hit no matter what
  }

  result->data = NULL;
  return false;
}

static bool selection_service_get_note_under_cursor_toggle(SelectionService *selection_service, RaycastNote *result)
{
  int length = selection_service->raycast_results.size;
  if (length > MAX_RAYCAST_RESULTS) length = MAX_RAYCAST_RESULTS;

  for (int i = 0; i < length; i++)
  {
    RaycastNote *ray_note = (RaycastNote *)list_get(&selection_service->raycast_results, i);
    if (!ray_note->collision.hit) continue;

    for (int j = 0; j < selection_service->selected_notes.size; j++)
    {
      struct SelectedNoteHolder *selected_note = (struct SelectedNoteHolder *)list_get(&selection_service->selected_notes, j);
      if (selected_note->data == ray_note->data) continue;

      result->data      = ray_note->data;
      result->note_type = ray_note->note_type;
      return true;
    }
  }

  for (int i = 0; i < length; i++)
  {
    RaycastNote *ray_note = (RaycastNote *)list_get(&selection_service->raycast_results, i);
    if (!ray_note->collision.hit) continue;

    result->data      = ray_note->data;
    result->note_type = ray_note->note_type;
    return true;
  }

  result->data = NULL;
  return false;
}

static void selection_service_reset_note_state(SelectionService *selection_service)
{
  for (int i = 0; i < selection_service->selected_notes.size; i++)
  {
    struct SelectedNoteHolder *selected_note = (struct SelectedNoteHolder *)list_get(&selection_service->selected_notes, i);
    switch (selected_note->note_type)
    {
      case TAP:    { Tap *tap        = (Tap *)selected_note->data       ; tap->is_selected    = false; break; }
      case HOLD:   { Hold *hold      = (Hold *)selected_note->data      ; hold->is_selected   = false; break; }
      case ARCTAP: { ArcTap *arctap  = (ArcTap *)selected_note->data    ; arctap->is_selected = false; break; }
      case ARC:    { struct Arc *arc = (struct Arc *)selected_note->data; arc->is_selected    = false; break; }
      default: break;
    }
  }
}

static void selection_service_update_note_state(SelectionService *selection_service)
{
  for (int i = 0; i < selection_service->selected_notes.size; i++)
  {
    struct SelectedNoteHolder *selected_note = (struct SelectedNoteHolder *)list_get(&selection_service->selected_notes, i);
    switch (selected_note->note_type)
    {
      case TAP:    { Tap *tap = (Tap *)selected_note->data              ; tap->is_selected    = true; break; }
      case HOLD:   { Hold *hold = (Hold *)selected_note->data           ; hold->is_selected   = true; break; }
      case ARCTAP: { ArcTap *arctap = (ArcTap *)selected_note->data     ; arctap->is_selected = true; break; }
      case ARC:    { struct Arc *arc = (struct Arc *)selected_note->data; arc->is_selected    = true; break; }
      default: break;
    }
  }
  printf("selected count: %zu\n", selection_service->selected_notes.size);
}

void selection_service_handle_inputs(SelectionService *selection_service, TrackService *track_service, WindowGroup *window_services, bool is_chart_initiailzed)
{
  if (!is_chart_initiailzed) return;
  bool has_update = false;
  static int range_select_tracker = -1;

  if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) &&
      IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) // Toggle select
  {
    if (window_services->project_setting.is_visible &&
        window_inst_bounded_cursor(&window_services->project_setting))
      return;

    RaycastNote note_result;
    if (!selection_service_get_note_under_cursor_toggle(selection_service, &note_result)) return;

    bool has_note = false;
    size_t del_idx = -1;
    for (int i = 0; i < selection_service->selected_notes.size; i++)
    {
      struct SelectedNoteHolder* selected_note = (struct SelectedNoteHolder *)list_get(&selection_service->selected_notes, i);
      if (selected_note->data != note_result.data) continue;

      has_note = true;
      del_idx  = (size_t)i;
      break;
    }

    selection_service_reset_note_state(selection_service);
    if (has_note) list_remove_last_swap(&selection_service->selected_notes, del_idx);
    else
    {
      struct SelectedNoteHolder holder = { .data = note_result.data, .note_type = note_result.note_type };
      list_push(&selection_service->selected_notes, &holder);
    }
    has_update = true;
  }
  else if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) // Single select
  {
    if (window_services->project_setting.is_visible &&
        window_inst_bounded_cursor(&window_services->project_setting))
      return;

    RaycastNote note_result;
    if (selection_service_get_note_under_cursor(selection_service, &note_result))
    {
      selection_service_reset_note_state(selection_service);
      list_clear(&selection_service->selected_notes);
      struct SelectedNoteHolder holder = { .data = note_result.data, .note_type = note_result.note_type };
      list_push(&selection_service->selected_notes, &holder);
      has_update = true;
    }
    else if (selection_service->selected_notes.size > 0)
    {
      selection_service_reset_note_state(selection_service);
      list_clear(&selection_service->selected_notes);
      has_update = true;
    }
  }

  if (has_update) selection_service_update_note_state(selection_service);
}

void selection_service_unload(SelectionService *selection_service)
{
  list_free(&selection_service->selected_notes);
  list_free(&selection_service->raycast_results);
}
