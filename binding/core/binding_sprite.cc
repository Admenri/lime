#include "binding_sprite.h"

#include "binding_bitmap.h"
#include "binding_color.h"
#include "binding_rect.h"
#include "binding_tone.h"
#include "binding_viewport.h"
#include "sprite.h"

namespace binding {

// Define mrb data type
MRB_DATATYPE_DEFINE(Sprite);

MRB_FUNC(Sprite_initialize) {
  mrb_value viewport_val = mrb_nil_value();
  mrb_get_args(mrb, "|o", &viewport_val);

  auto viewport = GetObject<rgssx::Viewport>(mrb, viewport_val, kViewportDataType);

  rgssx::RefPtr<rgssx::Sprite> obj = nullptr;
  EXC_BEGIN {
    obj = rgssx::MakeRefCounted<rgssx::Sprite>(viewport);
  } EXC_END(mrb);

  SetupSelfData(self, obj.get(), kSpriteDataType);
  return self;
}

MRB_FUNC(Sprite_Flash) {
  auto* self_obj = GetSelfData<rgssx::Sprite>(self);
  mrb_value color_val;
  mrb_int duration;
  mrb_get_args(mrb, "oi", &color_val, &duration);

  auto color = GetObject<rgssx::Color>(mrb, color_val, kColorDataType);

  EXC_BEGIN {
    self_obj->Flash(color, duration);
  } EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Sprite_Update) {
  auto* self_obj = GetSelfData<rgssx::Sprite>(self);
  EXC_BEGIN {
    self_obj->Update();
  } EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Sprite_Width) {
  auto* self_obj = GetSelfData<rgssx::Sprite>(self);
  EXC_BEGIN {
    return mrb_fixnum_value(self_obj->Width());
  } EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Sprite_Height) {
  auto* self_obj = GetSelfData<rgssx::Sprite>(self);
  EXC_BEGIN {
    return mrb_fixnum_value(self_obj->Height());
  } EXC_END(mrb);
  return mrb_nil_value();
}

BINDING_ATTR_OBJECT(Sprite, rgssx::Sprite, Bitmap, rgssx::Bitmap, kBitmapDataType);
BINDING_ATTR_OBJECT(Sprite, rgssx::Sprite, SrcRect, rgssx::Rect, kRectDataType);
BINDING_ATTR_INT(Sprite, rgssx::Sprite, X);
BINDING_ATTR_INT(Sprite, rgssx::Sprite, Y);
BINDING_ATTR_INT(Sprite, rgssx::Sprite, OX);
BINDING_ATTR_INT(Sprite, rgssx::Sprite, OY);
BINDING_ATTR_FLOAT(Sprite, rgssx::Sprite, ZoomX);
BINDING_ATTR_FLOAT(Sprite, rgssx::Sprite, ZoomY);
BINDING_ATTR_FLOAT(Sprite, rgssx::Sprite, Angle);
BINDING_ATTR_INT(Sprite, rgssx::Sprite, WaveAmp);
BINDING_ATTR_INT(Sprite, rgssx::Sprite, WaveLength);
BINDING_ATTR_INT(Sprite, rgssx::Sprite, WaveSpeed);
BINDING_ATTR_FLOAT(Sprite, rgssx::Sprite, WavePhase);
BINDING_ATTR_BOOL(Sprite, rgssx::Sprite, Mirror);
BINDING_ATTR_INT(Sprite, rgssx::Sprite, BushDepth);
BINDING_ATTR_INT(Sprite, rgssx::Sprite, BushOpacity);
BINDING_ATTR_INT(Sprite, rgssx::Sprite, Opacity);
BINDING_ATTR_INT(Sprite, rgssx::Sprite, BlendType);
BINDING_ATTR_OBJECT(Sprite, rgssx::Sprite, Color, rgssx::Color, kColorDataType);
BINDING_ATTR_OBJECT(Sprite, rgssx::Sprite, Tone, rgssx::Tone, kToneDataType);

// Inherited from Dispoable / ViewportChild / Drawable
BINDING_INHERITED_DISPOABLE(Sprite, rgssx::Sprite);
BINDING_ATTR_OBJECT(Sprite, rgssx::Sprite, Viewport, rgssx::Viewport, kViewportDataType);
BINDING_ATTR_BOOL(Sprite, rgssx::Sprite, Visible);
BINDING_ATTR_INT(Sprite, rgssx::Sprite, Z);

void InitSpriteBinding(mrb_state* mrb) {
  auto klass = DefineClass(mrb, "Sprite");

  mrb_define_method(mrb, klass, "initialize", Sprite_initialize, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "flash", Sprite_Flash, MRB_ARGS_REQ(2));
  mrb_define_method(mrb, klass, "update", Sprite_Update, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "width", Sprite_Width, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "height", Sprite_Height, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "bitmap", Sprite_Bitmap, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "bitmap=", Sprite_BitmapEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "src_rect", Sprite_SrcRect, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "src_rect=", Sprite_SrcRectEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "x", Sprite_X, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "x=", Sprite_XEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "y", Sprite_Y, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "y=", Sprite_YEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "ox", Sprite_OX, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "ox=", Sprite_OXEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "oy", Sprite_OY, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "oy=", Sprite_OYEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "zoom_x", Sprite_ZoomX, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "zoom_x=", Sprite_ZoomXEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "zoom_y", Sprite_ZoomY, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "zoom_y=", Sprite_ZoomYEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "angle", Sprite_Angle, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "angle=", Sprite_AngleEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "wave_amp", Sprite_WaveAmp, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "wave_amp=", Sprite_WaveAmpEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "wave_length", Sprite_WaveLength, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "wave_length=", Sprite_WaveLengthEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "wave_speed", Sprite_WaveSpeed, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "wave_speed=", Sprite_WaveSpeedEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "wave_phase", Sprite_WavePhase, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "wave_phase=", Sprite_WavePhaseEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "mirror", Sprite_Mirror, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "mirror=", Sprite_MirrorEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "bush_depth", Sprite_BushDepth, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "bush_depth=", Sprite_BushDepthEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "bush_opacity", Sprite_BushOpacity, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "bush_opacity=", Sprite_BushOpacityEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "opacity", Sprite_Opacity, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "opacity=", Sprite_OpacityEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "blend_type", Sprite_BlendType, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "blend_type=", Sprite_BlendTypeEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "color", Sprite_Color, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "color=", Sprite_ColorEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "tone", Sprite_Tone, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "tone=", Sprite_ToneEqual, MRB_ARGS_REQ(1));
  // Inherited from Dispoable
  mrb_define_method(mrb, klass, "dispose", Sprite_Dispose, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "is_disposed", Sprite_IsDisposed, MRB_ARGS_NONE());
  // Inherited from ViewportChild / Drawable
  mrb_define_method(mrb, klass, "viewport", Sprite_Viewport, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "viewport=", Sprite_ViewportEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "visible", Sprite_Visible, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "visible=", Sprite_VisibleEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "z", Sprite_Z, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "z=", Sprite_ZEqual, MRB_ARGS_REQ(1));
}

}  // namespace binding
