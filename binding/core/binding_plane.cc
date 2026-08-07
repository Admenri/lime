#include "binding_plane.h"

#include "binding_bitmap.h"
#include "binding_color.h"
#include "binding_tone.h"
#include "binding_viewport.h"

#include "src/plane.h"

namespace binding {

// Define mrb data type
MRB_DATATYPE_DEFINE(Plane);

MRB_FUNC(Plane_initialize) {
  mrb_value viewport_val = mrb_nil_value();
  mrb_get_args(mrb, "|o", &viewport_val);

  auto viewport = GetObject<rgssx::Viewport>(mrb, viewport_val, kViewportDataType);

  rgssx::RefPtr<rgssx::Plane> obj = nullptr;
  EXC_BEGIN {
    obj = rgssx::MakeRefCounted<rgssx::Plane>(viewport);
  } EXC_END(mrb);

  return SetupSelfData(self, obj.get(), kPlaneDataType);
}

BINDING_ATTR_OBJECT_REF(Plane, rgssx::Plane, Bitmap, rgssx::Bitmap, kBitmapDataType);
BINDING_ATTR_INT(Plane, rgssx::Plane, OX);
BINDING_ATTR_INT(Plane, rgssx::Plane, OY);
BINDING_ATTR_FLOAT(Plane, rgssx::Plane, ZoomX);
BINDING_ATTR_FLOAT(Plane, rgssx::Plane, ZoomY);
BINDING_ATTR_INT(Plane, rgssx::Plane, Opacity);
BINDING_ATTR_INT(Plane, rgssx::Plane, BlendType);
BINDING_ATTR_OBJECT_REF(Plane, rgssx::Plane, Color, rgssx::Color, kColorDataType);
BINDING_ATTR_OBJECT_REF(Plane, rgssx::Plane, Tone, rgssx::Tone, kToneDataType);

// Inherited from Dispoable / ViewportChild / Drawable
BINDING_INHERITED_DISPOABLE(Plane, rgssx::Plane);
BINDING_ATTR_OBJECT_REF(Plane, rgssx::Plane, Viewport, rgssx::Viewport, kViewportDataType);
BINDING_ATTR_BOOL(Plane, rgssx::Plane, Visible);
BINDING_ATTR_INT(Plane, rgssx::Plane, Z);

void InitPlaneBinding(mrb_state* mrb) {
  auto klass = DefineClass(mrb, "Plane");

  mrb_define_method(mrb, klass, "initialize", Plane_initialize, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "bitmap", Plane_Bitmap, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "bitmap=", Plane_BitmapEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "ox", Plane_OX, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "ox=", Plane_OXEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "oy", Plane_OY, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "oy=", Plane_OYEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "zoom_x", Plane_ZoomX, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "zoom_x=", Plane_ZoomXEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "zoom_y", Plane_ZoomY, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "zoom_y=", Plane_ZoomYEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "opacity", Plane_Opacity, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "opacity=", Plane_OpacityEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "blend_type", Plane_BlendType, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "blend_type=", Plane_BlendTypeEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "color", Plane_Color, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "color=", Plane_ColorEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "tone", Plane_Tone, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "tone=", Plane_ToneEqual, MRB_ARGS_REQ(1));
  // Inherited from Dispoable
  mrb_define_method(mrb, klass, "dispose", Plane_Dispose, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "disposed?", Plane_IsDisposed, MRB_ARGS_NONE());
  // Inherited from ViewportChild / Drawable
  mrb_define_method(mrb, klass, "viewport", Plane_Viewport, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "viewport=", Plane_ViewportEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "visible", Plane_Visible, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "visible=", Plane_VisibleEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "z", Plane_Z, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "z=", Plane_ZEqual, MRB_ARGS_REQ(1));
}

}  // namespace binding
