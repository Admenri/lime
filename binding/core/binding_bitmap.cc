#include "binding_bitmap.h"

#include "binding_color.h"
#include "binding_font.h"
#include "binding_rect.h"

#include "src/bitmap.h"
#include "src/utility.h"

namespace binding {

// Define mrb data type
MRB_DATATYPE_DEFINE(Bitmap);

MRB_FUNC(Bitmap_initialize) {
  const mrb_value* args;
  mrb_int argc;
  mrb_get_args(mrb, "*", &args, &argc);

  rgssx::RefPtr<rgssx::Bitmap> obj = nullptr;
  EXC_BEGIN {
    if (argc == 1) {
      // Bitmap.new(filename)
      std::string filename = mrb_str_to_cstr(mrb, args[0]);
      obj = rgssx::MakeRefCounted<rgssx::Bitmap>(filename);
    } else if (argc == 2) {
      // Bitmap.new(width, height)
      mrb_int width = mrb_integer(args[0]);
      mrb_int height = mrb_integer(args[1]);
      obj = rgssx::MakeRefCounted<rgssx::Bitmap>(width, height);
    } else {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
    }
  }
  EXC_END(mrb);

  SetupSelfData(self, obj.get(), kBitmapDataType);
  return self;
}

MRB_FUNC(Bitmap_Width) {
  auto* self_obj = GetSelfData<rgssx::Bitmap>(self);
  EXC_BEGIN {
    return mrb_fixnum_value(self_obj->Width());
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_Height) {
  auto* self_obj = GetSelfData<rgssx::Bitmap>(self);
  EXC_BEGIN {
    return mrb_fixnum_value(self_obj->Height());
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_GetRect) {
  auto* self_obj = GetSelfData<rgssx::Bitmap>(self);
  EXC_BEGIN {
    auto result = self_obj->GetRect();
    return WrapObject(mrb, result.get(), kRectDataType);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_Blt) {
  auto* self_obj = GetSelfData<rgssx::Bitmap>(self);
  mrb_int x, y, opacity = 255;
  mrb_value src_bitmap_val, src_rect_val;
  mrb_get_args(mrb, "iioo|i", &x, &y, &src_bitmap_val, &src_rect_val, &opacity);

  auto src_bitmap =
      GetObject<rgssx::Bitmap>(mrb, src_bitmap_val, kBitmapDataType);
  auto src_rect = GetObject<rgssx::Rect>(mrb, src_rect_val, kRectDataType);

  EXC_BEGIN {
    self_obj->Blt(x, y, src_bitmap, src_rect, opacity);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_StretchBlt) {
  auto* self_obj = GetSelfData<rgssx::Bitmap>(self);
  mrb_value dst_rect_val, src_bitmap_val, src_rect_val;
  mrb_int opacity = 255;
  mrb_get_args(mrb, "ooo|i", &dst_rect_val, &src_bitmap_val, &src_rect_val,
               &opacity);

  auto dst_rect = GetObject<rgssx::Rect>(mrb, dst_rect_val, kRectDataType);
  auto src_bitmap =
      GetObject<rgssx::Bitmap>(mrb, src_bitmap_val, kBitmapDataType);
  auto src_rect = GetObject<rgssx::Rect>(mrb, src_rect_val, kRectDataType);

  EXC_BEGIN {
    self_obj->StretchBlt(dst_rect, src_bitmap, src_rect, opacity);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_FillRect) {
  auto* self_obj = GetSelfData<rgssx::Bitmap>(self);
  const mrb_value* args;
  mrb_int argc;
  mrb_get_args(mrb, "*", &args, &argc);

  EXC_BEGIN {
    if (argc == 5) {
      // FillRect(x, y, width, height, color)
      self_obj->FillRect(mrb_integer(args[0]), mrb_integer(args[1]),
                         mrb_integer(args[2]), mrb_integer(args[3]),
                         GetObject<rgssx::Color>(mrb, args[4], kColorDataType));
    } else if (argc == 2) {
      // FillRect(rect, color)
      self_obj->FillRect(GetObject<rgssx::Rect>(mrb, args[0], kRectDataType),
                         GetObject<rgssx::Color>(mrb, args[1], kColorDataType));
    } else {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
    }
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_GradientFillRect) {
  auto* self_obj = GetSelfData<rgssx::Bitmap>(self);
  const mrb_value* args;
  mrb_int argc;
  mrb_get_args(mrb, "*", &args, &argc);

  EXC_BEGIN {
    if (argc == 3) {
      // GradientFillRect(rect, color1, color2)  (vertical = false)
      self_obj->GradientFillRect(
          GetObject<rgssx::Rect>(mrb, args[0], kRectDataType),
          GetObject<rgssx::Color>(mrb, args[1], kColorDataType),
          GetObject<rgssx::Color>(mrb, args[2], kColorDataType));
    } else if (argc == 4) {
      // GradientFillRect(rect, color1, color2, vertical)
      self_obj->GradientFillRect(
          GetObject<rgssx::Rect>(mrb, args[0], kRectDataType),
          GetObject<rgssx::Color>(mrb, args[1], kColorDataType),
          GetObject<rgssx::Color>(mrb, args[2], kColorDataType),
          mrb_test(args[3]));
    } else if (argc == 6) {
      // GradientFillRect(x, y, width, height, color1, color2)  (vertical =
      // false)
      self_obj->GradientFillRect(
          mrb_integer(args[0]), mrb_integer(args[1]), mrb_integer(args[2]),
          mrb_integer(args[3]),
          GetObject<rgssx::Color>(mrb, args[4], kColorDataType),
          GetObject<rgssx::Color>(mrb, args[5], kColorDataType));
    } else if (argc == 7) {
      // GradientFillRect(x, y, width, height, color1, color2, vertical)
      self_obj->GradientFillRect(
          mrb_integer(args[0]), mrb_integer(args[1]), mrb_integer(args[2]),
          mrb_integer(args[3]),
          GetObject<rgssx::Color>(mrb, args[4], kColorDataType),
          GetObject<rgssx::Color>(mrb, args[5], kColorDataType),
          mrb_test(args[6]));
    } else {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
    }
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_Clear) {
  auto* self_obj = GetSelfData<rgssx::Bitmap>(self);
  EXC_BEGIN {
    self_obj->Clear();
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_ClearRect) {
  auto* self_obj = GetSelfData<rgssx::Bitmap>(self);
  const mrb_value* args;
  mrb_int argc;
  mrb_get_args(mrb, "*", &args, &argc);

  EXC_BEGIN {
    if (argc == 4) {
      // ClearRect(x, y, width, height)
      self_obj->ClearRect(mrb_integer(args[0]), mrb_integer(args[1]),
                          mrb_integer(args[2]), mrb_integer(args[3]));
    } else if (argc == 1) {
      // ClearRect(rect)
      self_obj->ClearRect(GetObject<rgssx::Rect>(mrb, args[0], kRectDataType));
    } else {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
    }
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_GetPixel) {
  auto* self_obj = GetSelfData<rgssx::Bitmap>(self);
  mrb_int x, y;
  mrb_get_args(mrb, "ii", &x, &y);

  EXC_BEGIN {
    auto result = self_obj->GetPixel(x, y);
    return WrapObject(mrb, result.get(), kColorDataType);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_SetPixel) {
  auto* self_obj = GetSelfData<rgssx::Bitmap>(self);
  mrb_int x, y;
  mrb_value color_val;
  mrb_get_args(mrb, "iio", &x, &y, &color_val);

  auto color = GetObject<rgssx::Color>(mrb, color_val, kColorDataType);

  EXC_BEGIN {
    self_obj->SetPixel(x, y, color);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_HueChange) {
  auto* self_obj = GetSelfData<rgssx::Bitmap>(self);
  mrb_int hue;
  mrb_get_args(mrb, "i", &hue);

  EXC_BEGIN {
    self_obj->HueChange(hue);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_Blur) {
  auto* self_obj = GetSelfData<rgssx::Bitmap>(self);
  EXC_BEGIN {
    self_obj->Blur();
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_RadialBlur) {
  auto* self_obj = GetSelfData<rgssx::Bitmap>(self);
  mrb_int angle, division;
  mrb_get_args(mrb, "ii", &angle, &division);

  EXC_BEGIN {
    self_obj->RadialBlur(angle, division);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawText) {
  auto* self_obj = GetSelfData<rgssx::Bitmap>(self);

  EXC_BEGIN {
    mrb_int argc = mrb_get_argc(mrb);

    if (argc < 4) {
      mrb_value rect_obj;
      mrb_value str;
      mrb_int align = 0;
      mrb_get_args(mrb, "oo|i", &rect_obj, &str, &align);

      mrb_value str_raw = mrb_obj_as_string(mrb, str);
      std::string str_std(RSTRING_PTR(str_raw), RSTRING_LEN(str_raw));
      self_obj->DrawText(GetObject<rgssx::Rect>(mrb, rect_obj, kRectDataType),
                         str_std, align);
    } else {
      mrb_int x, y, w, h;
      mrb_value str;
      mrb_int align = 0;
      mrb_get_args(mrb, "iiiio|i", &x, &y, &w, &h, &str, &align);

      mrb_value str_raw = mrb_obj_as_string(mrb, str);
      std::string str_std(RSTRING_PTR(str_raw), RSTRING_LEN(str_raw));
      self_obj->DrawText(x, y, w, h, str_std, align);
    }
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_TextSize) {
  auto* self_obj = GetSelfData<rgssx::Bitmap>(self);
  const char* str;
  mrb_get_args(mrb, "z", &str);

  EXC_BEGIN {
    auto result = self_obj->TextSize(str);
    return WrapObject(mrb, result.get(), kRectDataType);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_SaveFile) {
  auto* self_obj = GetSelfData<rgssx::Bitmap>(self);
  const char* filename;
  mrb_get_args(mrb, "z", &filename);

  EXC_BEGIN {
    self_obj->SaveFile(filename);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

// Attribute: font (RefPtr<Font>)
BINDING_ATTR_OBJECT_REF(Bitmap,
                        rgssx::Bitmap,
                        Font,
                        rgssx::Font,
                        kFontDataType);

// Inherited from Dispoable
BINDING_INHERITED_DISPOABLE(Bitmap, rgssx::Bitmap);

void InitBitmapBinding(mrb_state* mrb) {
  auto klass = DefineClass(mrb, "Bitmap");

  mrb_define_method(mrb, klass, "initialize", Bitmap_initialize,
                    MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "width", Bitmap_Width, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "height", Bitmap_Height, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "rect", Bitmap_GetRect, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "blt", Bitmap_Blt, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "stretch_blt", Bitmap_StretchBlt,
                    MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "fill_rect", Bitmap_FillRect, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "gradient_fill_rect", Bitmap_GradientFillRect,
                    MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "clear", Bitmap_Clear, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "clear_rect", Bitmap_ClearRect, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "get_pixel", Bitmap_GetPixel, MRB_ARGS_REQ(2));
  mrb_define_method(mrb, klass, "set_pixel", Bitmap_SetPixel, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "hue_change", Bitmap_HueChange,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "blur", Bitmap_Blur, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "radial_blur", Bitmap_RadialBlur,
                    MRB_ARGS_REQ(2));
  mrb_define_method(mrb, klass, "draw_text", Bitmap_DrawText, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "text_size", Bitmap_TextSize, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "save_file", Bitmap_SaveFile, MRB_ARGS_REQ(1));
  // Attribute: font
  mrb_define_method(mrb, klass, "font", Bitmap_Font, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "font=", Bitmap_FontEqual, MRB_ARGS_REQ(1));
  // Inherited from Dispoable
  mrb_define_method(mrb, klass, "dispose", Bitmap_Dispose, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "disposed?", Bitmap_IsDisposed,
                    MRB_ARGS_NONE());
}

}  // namespace binding
