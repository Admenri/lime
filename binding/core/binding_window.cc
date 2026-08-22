#include "binding_window.h"

#include "binding_bitmap.h"
#include "binding_rect.h"
#include "binding_tone.h"
#include "binding_viewport.h"

#include "src/window.h"

namespace binding {

// Define mrb data type
MRB_DATATYPE_DEFINE(Window);

MRB_FUNC(Window_initialize) {
  mrb_int argc = mrb_get_argc(mrb);

  lime::RefPtr<lime::Window> obj = nullptr;
  EXC_BEGIN {
    if (argc == 4) {
      // Window.new(x, y, width, height)
      mrb_int x, y, w, h;
      mrb_get_args(mrb, "iiii", &x, &y, &w, &h);
      obj = lime::MakeRefCounted<lime::Window>(x, y, w, h);
    } else if (argc == 0) {
      // Window.new
      obj = lime::MakeRefCounted<lime::Window>();
    } else {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
    }
  }
  EXC_END(mrb);

  return SetupSelfData(self, obj.get(), kWindowDataType);
}

MRB_FUNC(Window_Update) {
  auto* self_obj = GetSelfData<lime::Window>(self);
  EXC_BEGIN {
    self_obj->Update();
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Window_Move) {
  auto* self_obj = GetSelfData<lime::Window>(self);
  mrb_int x, y, width, height;
  mrb_get_args(mrb, "iiii", &x, &y, &width, &height);

  EXC_BEGIN {
    self_obj->Move(x, y, width, height);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Window_Opened) {
  auto* self_obj = GetSelfData<lime::Window>(self);
  EXC_BEGIN {
    return mrb_bool_value(self_obj->Opened());
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Window_Closed) {
  auto* self_obj = GetSelfData<lime::Window>(self);
  EXC_BEGIN {
    return mrb_bool_value(self_obj->Closed());
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

BINDING_ATTR_OBJECT_REF(Window,
                        lime::Window,
                        WindowSkin,
                        lime::Bitmap,
                        kBitmapDataType);
BINDING_ATTR_OBJECT_REF(Window,
                        lime::Window,
                        Contents,
                        lime::Bitmap,
                        kBitmapDataType);
BINDING_ATTR_OBJECT_REF(Window,
                        lime::Window,
                        CursorRect,
                        lime::Rect,
                        kRectDataType);
BINDING_ATTR_BOOL(Window, lime::Window, Active);
BINDING_ATTR_BOOL(Window, lime::Window, ArrowsVisible);
BINDING_ATTR_BOOL(Window, lime::Window, Pause);
BINDING_ATTR_INT(Window, lime::Window, X);
BINDING_ATTR_INT(Window, lime::Window, Y);
BINDING_ATTR_INT(Window, lime::Window, Width);
BINDING_ATTR_INT(Window, lime::Window, Height);
BINDING_ATTR_INT(Window, lime::Window, OX);
BINDING_ATTR_INT(Window, lime::Window, OY);
BINDING_ATTR_INT(Window, lime::Window, Padding);
BINDING_ATTR_INT(Window, lime::Window, PaddingBottom);
BINDING_ATTR_INT(Window, lime::Window, Opacity);
BINDING_ATTR_INT(Window, lime::Window, BackOpacity);
BINDING_ATTR_INT(Window, lime::Window, ContentsOpacity);
BINDING_ATTR_INT(Window, lime::Window, Openness);
BINDING_ATTR_OBJECT_REF(Window, lime::Window, Tone, lime::Tone, kToneDataType);
BINDING_ATTR_INT(Window, lime::Window, Scale);

// Inherited from Dispoable / ViewportChild / Drawable
BINDING_INHERITED_DISPOABLE(Window, lime::Window);
BINDING_ATTR_OBJECT_REF(Window,
                        lime::Window,
                        Viewport,
                        lime::Viewport,
                        kViewportDataType);
BINDING_ATTR_BOOL(Window, lime::Window, Visible);
BINDING_ATTR_INT(Window, lime::Window, Z);

void InitWindowBinding(mrb_state* mrb) {
  auto klass = DefineClass(mrb, "WindowVX");

  mrb_define_method(mrb, klass, "initialize", Window_initialize,
                    MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "update", Window_Update, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "move", Window_Move, MRB_ARGS_REQ(4));
  mrb_define_method(mrb, klass, "open?", Window_Opened, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "close?", Window_Closed, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "windowskin", Window_WindowSkin,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "windowskin=", Window_WindowSkinEqual,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "contents", Window_Contents, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "contents=", Window_ContentsEqual,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "cursor_rect", Window_CursorRect,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "cursor_rect=", Window_CursorRectEqual,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "active", Window_Active, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "active=", Window_ActiveEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "arrows_visible", Window_ArrowsVisible,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "arrows_visible=", Window_ArrowsVisibleEqual,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "pause", Window_Pause, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "pause=", Window_PauseEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "x", Window_X, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "x=", Window_XEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "y", Window_Y, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "y=", Window_YEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "width", Window_Width, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "width=", Window_WidthEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "height", Window_Height, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "height=", Window_HeightEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "ox", Window_OX, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "ox=", Window_OXEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "oy", Window_OY, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "oy=", Window_OYEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "padding", Window_Padding, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "padding=", Window_PaddingEqual,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "padding_bottom", Window_PaddingBottom,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "padding_bottom=", Window_PaddingBottomEqual,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "opacity", Window_Opacity, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "opacity=", Window_OpacityEqual,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "back_opacity", Window_BackOpacity,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "back_opacity=", Window_BackOpacityEqual,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "contents_opacity", Window_ContentsOpacity,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, klass,
                    "contents_opacity=", Window_ContentsOpacityEqual,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "openness", Window_Openness, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "openness=", Window_OpennessEqual,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "tone", Window_Tone, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "tone=", Window_ToneEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "scale", Window_Scale, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "scale=", Window_ScaleEqual, MRB_ARGS_REQ(1));
  // Inherited from Dispoable
  mrb_define_method(mrb, klass, "dispose", Window_Dispose, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "disposed?", Window_IsDisposed,
                    MRB_ARGS_NONE());
  // Inherited from ViewportChild / Drawable
  mrb_define_method(mrb, klass, "viewport", Window_Viewport, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "viewport=", Window_ViewportEqual,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "visible", Window_Visible, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "visible=", Window_VisibleEqual,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "z", Window_Z, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "z=", Window_ZEqual, MRB_ARGS_REQ(1));
}

}  // namespace binding
