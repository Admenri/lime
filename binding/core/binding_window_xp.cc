#include "binding_window_xp.h"

#include "binding_bitmap.h"
#include "binding_rect.h"
#include "binding_viewport.h"

#include "src/window_xp.h"

namespace binding {

MRB_DATATYPE_DEFINE(WindowXP);

MRB_FUNC(WindowXP_initialize) {
  mrb_value viewport_val = mrb_nil_value();
  mrb_get_args(mrb, "|o", &viewport_val);
  auto viewport =
      GetObject<lime::Viewport>(mrb, viewport_val, kViewportDataType);

  lime::RefPtr<lime::WindowXP> obj = nullptr;
  EXC_BEGIN {
    obj = lime::MakeRefCounted<lime::WindowXP>(viewport);
  }
  EXC_END(mrb);
  return SetupSelfData(self, obj.get(), kWindowXPDataType);
}

MRB_FUNC(WindowXP_Update) {
  auto* self_obj = GetSelfData<lime::WindowXP>(self);
  EXC_BEGIN { self_obj->Update(); }
  EXC_END(mrb);
  return mrb_nil_value();
}

BINDING_ATTR_OBJECT(WindowXP, lime::WindowXP, WindowSkin, lime::Bitmap,
                    kBitmapDataType);
BINDING_ATTR_OBJECT(WindowXP, lime::WindowXP, Contents, lime::Bitmap,
                    kBitmapDataType);
BINDING_ATTR_BOOL(WindowXP, lime::WindowXP, Stretch);
BINDING_ATTR_OBJECT_REF(WindowXP, lime::WindowXP, CursorRect, lime::Rect,
                        kRectDataType);
BINDING_ATTR_BOOL(WindowXP, lime::WindowXP, Active);
BINDING_ATTR_BOOL(WindowXP, lime::WindowXP, Pause);
BINDING_ATTR_INT(WindowXP, lime::WindowXP, X);
BINDING_ATTR_INT(WindowXP, lime::WindowXP, Y);
BINDING_ATTR_INT(WindowXP, lime::WindowXP, Width);
BINDING_ATTR_INT(WindowXP, lime::WindowXP, Height);
BINDING_ATTR_INT(WindowXP, lime::WindowXP, OX);
BINDING_ATTR_INT(WindowXP, lime::WindowXP, OY);
BINDING_ATTR_INT(WindowXP, lime::WindowXP, Opacity);
BINDING_ATTR_INT(WindowXP, lime::WindowXP, BackOpacity);
BINDING_ATTR_INT(WindowXP, lime::WindowXP, ContentsOpacity);

// Inherited from ViewportChild / Drawable
BINDING_ATTR_OBJECT_REF(WindowXP, lime::WindowXP, Viewport, lime::Viewport,
                        kViewportDataType);
BINDING_ATTR_BOOL(WindowXP, lime::WindowXP, Visible);
BINDING_ATTR_INT(WindowXP, lime::WindowXP, Z);

BINDING_INHERITED_DISPOABLE(WindowXP, lime::WindowXP);

void InitWindowXPBinding(mrb_state* mrb) {
  auto klass = DefineClass(mrb, "WindowXP");
  mrb_define_method(mrb, klass, "initialize", WindowXP_initialize,
                    MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "update", WindowXP_Update, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "windowskin", WindowXP_WindowSkin,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "windowskin=", WindowXP_WindowSkinEqual,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "contents", WindowXP_Contents,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "contents=", WindowXP_ContentsEqual,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "stretch", WindowXP_Stretch, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "stretch=", WindowXP_StretchEqual,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "cursor_rect", WindowXP_CursorRect,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "cursor_rect=", WindowXP_CursorRectEqual,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "active", WindowXP_Active, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "active=", WindowXP_ActiveEqual,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "pause", WindowXP_Pause, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "pause=", WindowXP_PauseEqual,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "x", WindowXP_X, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "x=", WindowXP_XEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "y", WindowXP_Y, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "y=", WindowXP_YEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "width", WindowXP_Width, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "width=", WindowXP_WidthEqual,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "height", WindowXP_Height, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "height=", WindowXP_HeightEqual,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "ox", WindowXP_OX, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "ox=", WindowXP_OXEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "oy", WindowXP_OY, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "oy=", WindowXP_OYEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "opacity", WindowXP_Opacity, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "opacity=", WindowXP_OpacityEqual,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "back_opacity", WindowXP_BackOpacity,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "back_opacity=", WindowXP_BackOpacityEqual,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "contents_opacity", WindowXP_ContentsOpacity,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "contents_opacity=",
                    WindowXP_ContentsOpacityEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "viewport", WindowXP_Viewport,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "viewport=", WindowXP_ViewportEqual,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "visible", WindowXP_Visible,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "visible=", WindowXP_VisibleEqual,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "z", WindowXP_Z, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "z=", WindowXP_ZEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "dispose", WindowXP_Dispose,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "disposed?", WindowXP_IsDisposed,
                    MRB_ARGS_NONE());
}

}  // namespace binding