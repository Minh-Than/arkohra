#include "note_render_lists.h"
#include "data/gameplay_events/beatline.h"
#include "data/gameplay_events/tap.h"
#include "data/gameplay_events/arctap.h"
#include "data/keyframe/value_channel.h"

void render_lists_init(NoteRenderLists *render_lists)
{
  list_init(&render_lists->beatline_render_list, sizeof(BeatLine));
  list_init(&render_lists->hold_render_list, sizeof(const void *));
  list_init(&render_lists->tap_render_list, sizeof(TapFP));
  list_init(&render_lists->arc_render_list, sizeof(const void *));
  list_init(&render_lists->arccap_render_list, sizeof(const void *));
  list_init(&render_lists->arctap_render_list, sizeof(ArcTapFP));

  render_lists->enwidencamera_channel = value_channel_init();
}

void render_lists_clear(NoteRenderLists *render_lists)
{
  list_clear(&render_lists->beatline_render_list);
  list_clear(&render_lists->hold_render_list);
  list_clear(&render_lists->tap_render_list);
  list_clear(&render_lists->arc_render_list);
  list_clear(&render_lists->arccap_render_list);
  list_clear(&render_lists->arctap_render_list);

  list_clear(&render_lists->enwidencamera_channel.keyframes);
}

void render_lists_unload(NoteRenderLists *render_lists)
{
  list_free(&render_lists->beatline_render_list);
  list_free(&render_lists->hold_render_list);
  list_free(&render_lists->tap_render_list);
  list_free(&render_lists->arc_render_list);
  list_free(&render_lists->arccap_render_list);
  list_free(&render_lists->arctap_render_list);

  value_channel_unload(&render_lists->enwidencamera_channel);
}
