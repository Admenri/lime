#include "binding_viewport.h"

#include "binding_bitmap.h"
#include "binding_color.h"
#include "binding_effect.h"
#include "binding_rect.h"
#include "binding_tone.h"

#include "src/viewport.h"

namespace binding {

// Define mrb data type
MRB_DATATYPE_DEFINE(Viewport);

MRB_FUNC(Viewport_initialize) {
  mrb_int argc = mrb_get_argc(mrb);

  lime::RefPtr<lime::Viewport> obj = nullptr;
  EXC_BEGIN {
    if (argc == 0) {
      // Viewport.new()
      obj = lime::MakeRefCounted<lime::Viewport>();
    } else if (argc == 1) {
      // Viewport.new(rect)
      mrb_value rect;
      mrb_get_args(mrb, "o", &rect);
      obj = lime::MakeRefCounted<lime::Viewport>(
          GetObject<lime::Rect>(mrb, rect, kRectDataType));
    } else if (argc == 2) {
      // Viewport.new(viewport, rect)
      mrb_value viewport, rect;
      mrb_get_args(mrb, "oo", &viewport, &rect);
      obj = lime::MakeRefCounted<lime::Viewport>(
          GetObject<lime::Viewport>(mrb, viewport, kViewportDataType),
          GetObject<lime::Rect>(mrb, rect, kRectDataType));
    } else if (argc == 4) {
      // Viewport.new(x, y, width, height)
      mrb_int x, y, w, h;
      mrb_get_args(mrb, "iiii", &x, &y, &w, &h);
      obj = lime::MakeRefCounted<lime::Viewport>(x, y, w, h);
    } else if (argc == 5) {
      // Viewport.new(viewport, x, y, width, height)
      mrb_value viewport;
      mrb_int x, y, w, h;
      mrb_get_args(mrb, "oiiii", &viewport, &x, &y, &w, &h);
      obj = lime::MakeRefCounted<lime::Viewport>(
          GetObject<lime::Viewport>(mrb, viewport, kViewportDataType), x, y, w,
          h);
    } else {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
    }
  }
  EXC_END(mrb);

  return SetupSelfData(self, obj.get(), kViewportDataType);
}

MRB_FUNC(Viewport_Flash) {
  auto* self_obj = GetSelfData<lime::Viewport>(self);
  mrb_value color_val;
  mrb_int duration;
  mrb_get_args(mrb, "oi", &color_val, &duration);

  auto color = GetObject<lime::Color>(mrb, color_val, kColorDataType);

  EXC_BEGIN {
    self_obj->Flash(color, duration);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Viewport_Update) {
  auto* self_obj = GetSelfData<lime::Viewport>(self);
  EXC_BEGIN {
    self_obj->Update();
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Viewport_Render) {
  auto* self_obj = GetSelfData<lime::Viewport>(self);
  mrb_value bitmap_val;
  mrb_get_args(mrb, "o", &bitmap_val);

  auto bitmap = GetObject<lime::Bitmap>(mrb, bitmap_val, kBitmapDataType);

  EXC_BEGIN {
    self_obj->Render(bitmap);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

BINDING_ATTR_OBJECT_REF(Viewport,
                        lime::Viewport,
                        Rect,
                        lime::Rect,
                        kRectDataType);
BINDING_ATTR_INT(Viewport, lime::Viewport, OX);
BINDING_ATTR_INT(Viewport, lime::Viewport, OY);
BINDING_ATTR_FLOAT(Viewport, lime::Viewport, Angle);
BINDING_ATTR_FLOAT(Viewport, lime::Viewport, ZoomX);
BINDING_ATTR_FLOAT(Viewport, lime::Viewport, ZoomY);
BINDING_ATTR_BOOL(Viewport, lime::Viewport, Clip);
BINDING_ATTR_OBJECT_REF(Viewport,
                        lime::Viewport,
                        Color,
                        lime::Color,
                        kColorDataType);
BINDING_ATTR_OBJECT_REF(Viewport,
                        lime::Viewport,
                        Tone,
                        lime::Tone,
                        kToneDataType);

// Inherited from Dispoable / ViewportChild / Drawable
BINDING_INHERITED_DISPOABLE(Viewport, lime::Viewport);
BINDING_ATTR_OBJECT_REF(Viewport,
                        lime::Viewport,
                        Viewport,
                        lime::Viewport,
                        kViewportDataType);
BINDING_ATTR_BOOL(Viewport, lime::Viewport, Visible);
BINDING_ATTR_INT(Viewport, lime::Viewport, Z);

void InitViewportBinding(mrb_state* mrb) {
  auto klass = DefineClass(mrb, "Viewport");

  mrb_define_method(mrb, klass, "initialize", Viewport_initialize,
                    MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "flash", Viewport_Flash, MRB_ARGS_REQ(2));
  mrb_define_method(mrb, klass, "update", Viewport_Update, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "render", Viewport_Render, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "rect", Viewport_Rect, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "rect=", Viewport_RectEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "ox", Viewport_OX, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "ox=", Viewport_OXEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "oy", Viewport_OY, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "oy=", Viewport_OYEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "angle", Viewport_Angle, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "angle=", Viewport_AngleEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "zoom_x", Viewport_ZoomX, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "zoom_x=", Viewport_ZoomXEqual,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "zoom_y", Viewport_ZoomY, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "zoom_y=", Viewport_ZoomYEqual,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "clip", Viewport_Clip, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "clip=", Viewport_ClipEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "color", Viewport_Color, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "color=", Viewport_ColorEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "tone", Viewport_Tone, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "tone=", Viewport_ToneEqual, MRB_ARGS_REQ(1));
  // Inherited from Dispoable
  mrb_define_method(mrb, klass, "dispose", Viewport_Dispose, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "disposed?", Viewport_IsDisposed,
                    MRB_ARGS_NONE());
  // Inherited from ViewportChild / Drawable
  mrb_define_method(mrb, klass, "viewport", Viewport_Viewport, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "viewport=", Viewport_ViewportEqual,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "visible", Viewport_Visible, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "visible=", Viewport_VisibleEqual,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "z", Viewport_Z, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "z=", Viewport_ZEqual, MRB_ARGS_REQ(1));
}

}  // namespace binding
