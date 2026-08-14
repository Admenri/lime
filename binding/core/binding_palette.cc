#include "binding_palette.h"

#include "binding_color.h"

#include "src/palette.h"

namespace binding {

// Define mrb data type
MRB_DATATYPE_DEFINE(Palette);

MRB_FUNC(Palette_initialize) {
  mrb_int argc = mrb_get_argc(mrb);

  lime::RefPtr<lime::Palette> obj = nullptr;
  EXC_BEGIN {
    if (argc == 1) {
      // Palette.new(filename)
      const char* filename;
      mrb_get_args(mrb, "z", &filename);
      obj = lime::MakeRefCounted<lime::Palette>(filename);
    } else if (argc == 2) {
      // Palette.new(width, height)
      mrb_int width, height;
      mrb_get_args(mrb, "ii", &width, &height);
      obj = lime::MakeRefCounted<lime::Palette>(width, height);
    } else {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
    }
  }
  EXC_END(mrb);

  return SetupSelfData(self, obj.get(), kPaletteDataType);
}

MRB_FUNC(Palette_GetPixel) {
  auto* self_obj = GetSelfData<lime::Palette>(self);
  mrb_int x, y;
  mrb_get_args(mrb, "ii", &x, &y);

  EXC_BEGIN {
    auto result = self_obj->GetPixel(x, y);
    return WrapObject(mrb, result.get(), kColorDataType);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Palette_SetPixel) {
  auto* self_obj = GetSelfData<lime::Palette>(self);
  mrb_int x, y;
  mrb_value color_val;
  mrb_get_args(mrb, "iio", &x, &y, &color_val);

  auto color = GetObject<lime::Color>(mrb, color_val, kColorDataType);

  EXC_BEGIN {
    self_obj->SetPixel(x, y, color);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Palette_SaveFile) {
  auto* self_obj = GetSelfData<lime::Palette>(self);
  const char* filename;
  mrb_get_args(mrb, "z", &filename);

  EXC_BEGIN {
    self_obj->SaveFile(filename);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

// Inherited from Dispoable
BINDING_INHERITED_DISPOABLE(Palette, lime::Palette);

void InitPaletteBinding(mrb_state* mrb) {
  auto klass = DefineClass(mrb, "Palette");

  mrb_define_method(mrb, klass, "initialize", Palette_initialize,
                    MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "get_pixel", Palette_GetPixel, MRB_ARGS_REQ(2));
  mrb_define_method(mrb, klass, "set_pixel", Palette_SetPixel, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "save_file", Palette_SaveFile,
                    MRB_ARGS_REQ(1));
  // Inherited from Dispoable
  mrb_define_method(mrb, klass, "dispose", Palette_Dispose, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "disposed?", Palette_IsDisposed,
                    MRB_ARGS_NONE());
}

}  // namespace binding
