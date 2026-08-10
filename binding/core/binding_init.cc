#include "binding_init.h"

#include "binding_audio.h"
#include "binding_bitmap.h"
#include "binding_color.h"
#include "binding_font.h"
#include "binding_geometry.h"
#include "binding_graphics.h"
#include "binding_input.h"
#include "binding_plane.h"
#include "binding_rect.h"
#include "binding_shader.h"
#include "binding_sprite.h"
#include "binding_table.h"
#include "binding_tilemap.h"
#include "binding_tone.h"
#include "binding_viewport.h"
#include "binding_window.h"

namespace binding {

void InitBindings(mrb_state* mrb) {
  InitAudioBinding(mrb);
  InitBitmapBinding(mrb);
  InitColorBinding(mrb);
  InitFontBinding(mrb);
  InitGeometryBinding(mrb);
  InitGraphicsBinding(mrb);
  InitInputBinding(mrb);
  InitPlaneBinding(mrb);
  InitRectBinding(mrb);
  InitShaderBinding(mrb);
  InitSpriteBinding(mrb);
  InitTableBinding(mrb);
  InitTilemapBinding(mrb);
  InitToneBinding(mrb);
  InitViewportBinding(mrb);
  InitWindowBinding(mrb);
}

}  // namespace binding
