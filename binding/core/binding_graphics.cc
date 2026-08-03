#include "binding_graphics.h"

#include "binding_bitmap.h"
#include "graphics.h"

namespace binding {

MRB_FUNC(Graphics_Update) {
  auto* self_obj = rgssx::Graphics::Instance();
  EXC_BEGIN {
    self_obj->Update();
  }
  EXC_END(mrb);
  if (raylib::WindowShouldClose())
    mrb_raise(mrb, mrb->eException_class, "exit");
  return mrb_nil_value();
}

MRB_FUNC(Graphics_Wait) {
  auto* self_obj = rgssx::Graphics::Instance();
  mrb_int duration;
  mrb_get_args(mrb, "i", &duration);

  EXC_BEGIN {
    self_obj->Wait(duration);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Graphics_FadeIn) {
  auto* self_obj = rgssx::Graphics::Instance();
  mrb_int duration;
  mrb_get_args(mrb, "i", &duration);

  EXC_BEGIN {
    self_obj->FadeIn(duration);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Graphics_FadeOut) {
  auto* self_obj = rgssx::Graphics::Instance();
  mrb_int duration;
  mrb_get_args(mrb, "i", &duration);

  EXC_BEGIN {
    self_obj->FadeOut(duration);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Graphics_Freeze) {
  auto* self_obj = rgssx::Graphics::Instance();
  EXC_BEGIN {
    self_obj->Freeze();
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Graphics_Transition) {
  auto* self_obj = rgssx::Graphics::Instance();
  const mrb_value* args;
  mrb_int argc;
  mrb_get_args(mrb, "*", &args, &argc);

  mrb_int duration = 10;
  std::string filename;
  mrb_int vague = 40;
  if (argc >= 1)
    duration = mrb_integer(args[0]);
  if (argc >= 2 && !mrb_nil_p(args[1]))
    filename = mrb_str_to_cstr(mrb, args[1]);
  if (argc >= 3)
    vague = mrb_integer(args[2]);

  EXC_BEGIN {
    self_obj->Transition(duration, filename, vague);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Graphics_TransitionBitmap) {
  auto* self_obj = rgssx::Graphics::Instance();
  const mrb_value* args;
  mrb_int argc;
  mrb_get_args(mrb, "*", &args, &argc);

  mrb_int duration = 10;
  mrb_value bitmap_val = mrb_nil_value();
  mrb_int vague = 40;
  if (argc >= 1)
    duration = mrb_integer(args[0]);
  if (argc >= 2)
    bitmap_val = args[1];
  if (argc >= 3)
    vague = mrb_integer(args[2]);

  auto bitmap = GetObject<rgssx::Bitmap>(mrb, bitmap_val, kBitmapDataType);

  EXC_BEGIN {
    self_obj->TransitionBitmap(duration, bitmap, vague);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Graphics_SnapToBitmap) {
  auto* self_obj = rgssx::Graphics::Instance();
  EXC_BEGIN {
    auto result = self_obj->SnapToBitmap();
    return WrapObject(mrb, result.get(), kBitmapDataType);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Graphics_FrameReset) {
  auto* self_obj = rgssx::Graphics::Instance();
  EXC_BEGIN {
    self_obj->FrameReset();
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Graphics_Width) {
  auto* self_obj = rgssx::Graphics::Instance();
  EXC_BEGIN {
    return mrb_fixnum_value(self_obj->Width());
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Graphics_Height) {
  auto* self_obj = rgssx::Graphics::Instance();
  EXC_BEGIN {
    return mrb_fixnum_value(self_obj->Height());
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Graphics_ResizeScreen) {
  auto* self_obj = rgssx::Graphics::Instance();
  mrb_int width, height;
  mrb_get_args(mrb, "ii", &width, &height);

  EXC_BEGIN {
    self_obj->ResizeScreen(width, height);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Graphics_PlayMovie) {
  auto* self_obj = rgssx::Graphics::Instance();
  const char* filename;
  mrb_get_args(mrb, "z", &filename);

  EXC_BEGIN {
    self_obj->PlayMovie(filename);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Graphics_WindowHandle) {
  auto* self_obj = rgssx::Graphics::Instance();
  EXC_BEGIN {
    return mrb_cptr_value(mrb, self_obj->WindowHandle());
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

#define GRAPHICS_INT_ATTR(cap)                       \
  MRB_FUNC(Graphics_##cap) {                         \
    auto* self_obj = rgssx::Graphics::Instance();    \
    EXC_BEGIN {                                      \
      auto result = self_obj->Attr_##cap();          \
      if (result.has_value())                        \
        return mrb_fixnum_value(*result);            \
      return mrb_nil_value();                        \
    }                                                \
    EXC_END(mrb);                                    \
    return mrb_nil_value();                          \
  }                                                  \
  MRB_FUNC(Graphics_##cap##Equal) {                  \
    auto* self_obj = rgssx::Graphics::Instance();    \
    mrb_int value;                                   \
    mrb_get_args(mrb, "i", &value);                  \
    EXC_BEGIN {                                      \
      self_obj->Attr_##cap(static_cast<int>(value)); \
    }                                                \
    EXC_END(mrb);                                    \
    return mrb_nil_value();                          \
  }

GRAPHICS_INT_ATTR(FrameRate);
GRAPHICS_INT_ATTR(FrameCount);
GRAPHICS_INT_ATTR(Brightness);

#undef GRAPHICS_INT_ATTR

void InitGraphicsBinding(mrb_state* mrb) {
  auto mod = mrb_define_module(mrb, "Graphics");

  mrb_define_module_function(mrb, mod, "update", Graphics_Update,
                             MRB_ARGS_NONE());
  mrb_define_module_function(mrb, mod, "wait", Graphics_Wait, MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, mod, "fadein", Graphics_FadeIn,
                             MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, mod, "fadeout", Graphics_FadeOut,
                             MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, mod, "freeze", Graphics_Freeze,
                             MRB_ARGS_NONE());
  mrb_define_module_function(mrb, mod, "transition", Graphics_Transition,
                             MRB_ARGS_ANY());
  mrb_define_module_function(mrb, mod, "transition_bitmap",
                             Graphics_TransitionBitmap, MRB_ARGS_ANY());
  mrb_define_module_function(mrb, mod, "snap_to_bitmap", Graphics_SnapToBitmap,
                             MRB_ARGS_NONE());
  mrb_define_module_function(mrb, mod, "frame_reset", Graphics_FrameReset,
                             MRB_ARGS_NONE());
  mrb_define_module_function(mrb, mod, "width", Graphics_Width,
                             MRB_ARGS_NONE());
  mrb_define_module_function(mrb, mod, "height", Graphics_Height,
                             MRB_ARGS_NONE());
  mrb_define_module_function(mrb, mod, "resize_screen", Graphics_ResizeScreen,
                             MRB_ARGS_REQ(2));
  mrb_define_module_function(mrb, mod, "play_movie", Graphics_PlayMovie,
                             MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, mod, "window_handle", Graphics_WindowHandle,
                             MRB_ARGS_NONE());
  mrb_define_module_function(mrb, mod, "frame_rate", Graphics_FrameRate,
                             MRB_ARGS_NONE());
  mrb_define_module_function(mrb, mod, "frame_rate=", Graphics_FrameRateEqual,
                             MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, mod, "frame_count", Graphics_FrameCount,
                             MRB_ARGS_NONE());
  mrb_define_module_function(mrb, mod, "frame_count=", Graphics_FrameCountEqual,
                             MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, mod, "brightness", Graphics_Brightness,
                             MRB_ARGS_NONE());
  mrb_define_module_function(mrb, mod, "brightness=", Graphics_BrightnessEqual,
                             MRB_ARGS_REQ(1));
}

}  // namespace binding
