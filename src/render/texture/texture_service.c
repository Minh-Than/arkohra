#include "texture_service.h"
#include "raylib.h"

TextureGroup textures_init()
{
  Texture2D background_texture      = LoadTexture("resources/gameplay/DefaultBackgrounds/arccreate-blender2_base_light.jpg");
  Texture2D kohracube_texture       = LoadTexture("resources/gameplay/kohar.png");
  Texture2D track_texture           = LoadTexture("resources/gameplay/Track/TrackWhite.png");
  Texture2D lane_div_texture        = LoadTexture("resources/gameplay/Track/TrackLaneDivider.png");
  Texture2D critical_line_texture   = LoadTexture("resources/gameplay/CriticalLine/TrackCriticalLine.png");
  Texture2D sky_input_line_texture  = LoadTexture("resources/gameplay/CriticalLine/SkyInputLine.png");
  Texture2D sky_label_texture       = LoadTexture("resources/gameplay/CriticalLine/SkyInputLabel.png");
  Texture2D single_line_texture     = LoadTexture("resources/gameplay/SingleLine/SingleLineNone.png");
  Texture2D pause_button_texture    = LoadTexture("resources/gameplay/HUD/PauseLight.png");
  Texture2D info_panel_texture      = LoadTexture("resources/gameplay/HUD/InfoLight.png");
  Texture2D jacket_bg_texture       = LoadTexture("resources/gameplay/HUD/JacketBackground.png");
  Texture2D jacket_img_texture      = LoadTexture("resources/gameplay/DefaultJacket.png");
  Texture2D jacket_diff_texture     = LoadTexture("resources/gameplay/HUD/Difficulty.png");

  Texture2D tap_texture             = LoadTexture("resources/gameplay/Note/Light/TapNoteLight.png");
  Texture2D hold_texture            = LoadTexture("resources/gameplay/Note/Light/HoldNoteLight.png");
  Texture2D arc_texture             = LoadTexture("resources/gameplay/Note/ArcBody.png");
  Texture2D arc_height_ind_texture  = LoadTexture("resources/gameplay/Note/HeightIndicator.png");
  Texture2D arccap_texture          = LoadTexture("resources/gameplay/Note/ArcCap.png");
  Texture2D arctap_texture          = LoadTexture("resources/gameplay/Note/Light/ArcTapLight.png");
  Texture2D arctap_shadow_texture   = LoadTexture("resources/gameplay/Note/ArcTapShadow.png");

  GenTextureMipmaps(&background_texture);
  SetTextureFilter(background_texture, TEXTURE_FILTER_TRILINEAR);
  GenTextureMipmaps(&kohracube_texture);
  SetTextureFilter(kohracube_texture, TEXTURE_FILTER_TRILINEAR);
  SetTextureWrap(track_texture, TEXTURE_WRAP_REPEAT);
  SetTextureFilter(lane_div_texture, TEXTURE_FILTER_BILINEAR);
  SetTextureFilter(lane_div_texture, TEXTURE_FILTER_BILINEAR);
  SetTextureWrap(single_line_texture, TEXTURE_WRAP_REPEAT);
  SetTextureFilter(jacket_img_texture, TEXTURE_FILTER_BILINEAR);
  SetTextureFilter(tap_texture, TEXTURE_FILTER_BILINEAR);
  SetTextureFilter(hold_texture, TEXTURE_FILTER_BILINEAR);
  SetTextureWrap(hold_texture, TEXTURE_WRAP_CLAMP);
  SetTextureFilter(arc_texture, TEXTURE_FILTER_BILINEAR);
  SetTextureWrap(arc_texture, TEXTURE_WRAP_CLAMP);

  return (TextureGroup){
    .background     = background_texture,
    .kohra          = kohracube_texture,
    .track          = track_texture,
    .lane_div       = lane_div_texture,
    .critical_line  = critical_line_texture,
    .sky_input_line = sky_input_line_texture,
    .sky_label      = sky_label_texture,
    .single_line    = single_line_texture,
    .pause_button   = pause_button_texture,
    .info_panel     = info_panel_texture,
    .jacket_bg      = jacket_bg_texture,
    .jacket_img     = jacket_img_texture,
    .jacket_diff    = jacket_diff_texture,
    .tap            = tap_texture,
    .hold           = hold_texture,
    .arc            = arc_texture,
    .arc_height_indicator = arc_height_ind_texture,
    .arccap         = arccap_texture,
    .arctap         = arctap_texture,
    .arctap_shadow  = arctap_shadow_texture
  };
}

void textures_unload(TextureGroup* texture_group)
{
  UnloadTexture(texture_group->background);
  UnloadTexture(texture_group->kohra);
  UnloadTexture(texture_group->track);
  UnloadTexture(texture_group->lane_div);
  UnloadTexture(texture_group->critical_line);
  UnloadTexture(texture_group->sky_input_line);
  UnloadTexture(texture_group->sky_label);
  UnloadTexture(texture_group->single_line);
  UnloadTexture(texture_group->pause_button);
  UnloadTexture(texture_group->info_panel);
  UnloadTexture(texture_group->jacket_bg);
  UnloadTexture(texture_group->jacket_img);
  UnloadTexture(texture_group->jacket_diff);
  UnloadTexture(texture_group->tap);
  UnloadTexture(texture_group->hold);
  UnloadTexture(texture_group->arc);
  UnloadTexture(texture_group->arc_height_indicator);
  UnloadTexture(texture_group->arccap);
  UnloadTexture(texture_group->arctap);
  UnloadTexture(texture_group->arctap_shadow);
}
