#include "binding_init.h"

#include "binding_audio.h"
#include "binding_bitmap.h"
#include "binding_color.h"
#include "binding_effect.h"
#include "binding_font.h"
#include "binding_geometry.h"
#include "binding_graphics.h"
#include "binding_input.h"
#include "binding_palette.h"
#include "binding_plane.h"
#include "binding_rect.h"
#include "binding_sprite.h"
#include "binding_table.h"
#include "binding_tilemap.h"
#include "binding_tilemap_xp.h"
#include "binding_tone.h"
#include "binding_vector.h"
#include "binding_viewport.h"
#include "binding_window.h"
#include "binding_window_xp.h"

#include "src/profile.h"

namespace binding {

void InitBindings(mrb_state* mrb) {
  InitAudioBinding(mrb);
  InitBitmapBinding(mrb);
  InitColorBinding(mrb);
  InitEffectBinding(mrb);
  InitFontBinding(mrb);
  InitGeometryBinding(mrb);
  InitGraphicsBinding(mrb);
  InitInputBinding(mrb);
  InitPaletteBinding(mrb);
  InitPlaneBinding(mrb);
  InitRectBinding(mrb);
  InitSpriteBinding(mrb);
  InitTableBinding(mrb);
  InitTilemapBinding(mrb);
  InitTilemapXPBinding(mrb);
  InitToneBinding(mrb);
  InitVectorBinding(mrb);
  InitViewportBinding(mrb);
  InitWindowBinding(mrb);
  InitWindowXPBinding(mrb);

  auto* tilemap_klass = mrb_class_get(
      mrb, lime::Config::Instance()->xp() ? "TilemapXP" : "TilemapVX");
  auto* window_klass = mrb_class_get(
      mrb, lime::Config::Instance()->xp() ? "WindowXP" : "WindowVX");
  mrb_define_const(mrb, mrb->kernel_module, "Tilemap",
                   mrb_obj_value(tilemap_klass));
  mrb_define_const(mrb, mrb->kernel_module, "Window",
                   mrb_obj_value(window_klass));
}

}  // namespace binding
