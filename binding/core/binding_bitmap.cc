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
  mrb_int argc = mrb_get_argc(mrb);

  lime::RefPtr<lime::Bitmap> obj = nullptr;
  EXC_BEGIN {
    if (argc == 1) {
      // Bitmap.new(filename)
      const char* filename;
      mrb_get_args(mrb, "z", &filename);
      obj = lime::MakeRefCounted<lime::Bitmap>(filename);
    } else if (argc == 2) {
      // Bitmap.new(width, height)
      mrb_int width, height;
      mrb_get_args(mrb, "ii", &width, &height);
      obj = lime::MakeRefCounted<lime::Bitmap>(width, height);
    } else {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
    }
  }
  EXC_END(mrb);

  return SetupSelfData(self, obj.get(), kBitmapDataType);
}

MRB_FUNC(Bitmap_initialize_copy) {
  mrb_value other;
  mrb_get_args(mrb, "o", &other);

  lime::RefPtr<lime::Bitmap> obj = nullptr;
  EXC_BEGIN {
    auto other_obj = GetObject<lime::Bitmap>(mrb, other, kBitmapDataType);
    obj = lime::MakeRefCounted<lime::Bitmap>(other_obj);
  }
  EXC_END(mrb);

  return SetupSelfData(self, obj.get(), kBitmapDataType);
}

MRB_FUNC(Bitmap_Width) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  EXC_BEGIN {
    return mrb_fixnum_value(self_obj->Width());
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_Height) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  EXC_BEGIN {
    return mrb_fixnum_value(self_obj->Height());
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_GetRect) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  EXC_BEGIN {
    auto result = self_obj->GetRect();
    return WrapObject(mrb, result.get(), kRectDataType);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_Blt) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_int x, y, opacity = 255;
  mrb_value src_bitmap_val, src_rect_val;
  mrb_get_args(mrb, "iioo|i", &x, &y, &src_bitmap_val, &src_rect_val, &opacity);

  auto src_bitmap =
      GetObject<lime::Bitmap>(mrb, src_bitmap_val, kBitmapDataType);
  auto src_rect = GetObject<lime::Rect>(mrb, src_rect_val, kRectDataType);

  EXC_BEGIN {
    self_obj->Blt(x, y, src_bitmap, src_rect, opacity);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_StretchBlt) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_value dst_rect_val, src_bitmap_val, src_rect_val;
  mrb_int opacity = 255;
  mrb_get_args(mrb, "ooo|i", &dst_rect_val, &src_bitmap_val, &src_rect_val,
               &opacity);

  auto dst_rect = GetObject<lime::Rect>(mrb, dst_rect_val, kRectDataType);
  auto src_bitmap =
      GetObject<lime::Bitmap>(mrb, src_bitmap_val, kBitmapDataType);
  auto src_rect = GetObject<lime::Rect>(mrb, src_rect_val, kRectDataType);

  EXC_BEGIN {
    self_obj->StretchBlt(dst_rect, src_bitmap, src_rect, opacity);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_FillRect) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);

  mrb_int argc = mrb_get_argc(mrb);

  EXC_BEGIN {
    if (argc == 5) {
      // FillRect(x, y, width, height, color)
      mrb_int x, y, width, height;
      mrb_value color_val;
      mrb_get_args(mrb, "iiiio", &x, &y, &width, &height, &color_val);
      self_obj->FillRect(
          x, y, width, height,
          GetObject<lime::Color>(mrb, color_val, kColorDataType));
    } else if (argc == 2) {
      // FillRect(rect, color)
      mrb_value rect_val, color_val;
      mrb_get_args(mrb, "oo", &rect_val, &color_val);
      self_obj->FillRect(
          GetObject<lime::Rect>(mrb, rect_val, kRectDataType),
          GetObject<lime::Color>(mrb, color_val, kColorDataType));
    } else {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
    }
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_GradientFillRect) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);

  mrb_int argc = mrb_get_argc(mrb);

  EXC_BEGIN {
    if (argc == 3) {
      // GradientFillRect(rect, color1, color2)  (vertical = false)
      mrb_value rect_val, color1_val, color2_val;
      mrb_get_args(mrb, "ooo", &rect_val, &color1_val, &color2_val);
      self_obj->GradientFillRect(
          GetObject<lime::Rect>(mrb, rect_val, kRectDataType),
          GetObject<lime::Color>(mrb, color1_val, kColorDataType),
          GetObject<lime::Color>(mrb, color2_val, kColorDataType));
    } else if (argc == 4) {
      // GradientFillRect(rect, color1, color2, vertical)
      mrb_value rect_val, color1_val, color2_val;
      mrb_bool vertical;
      mrb_get_args(mrb, "ooob", &rect_val, &color1_val, &color2_val, &vertical);
      self_obj->GradientFillRect(
          GetObject<lime::Rect>(mrb, rect_val, kRectDataType),
          GetObject<lime::Color>(mrb, color1_val, kColorDataType),
          GetObject<lime::Color>(mrb, color2_val, kColorDataType), vertical);
    } else if (argc == 6) {
      // GradientFillRect(x, y, width, height, color1, color2)  (vertical =
      // false)
      mrb_int x, y, width, height;
      mrb_value color1_val, color2_val;
      mrb_get_args(mrb, "iiiioo", &x, &y, &width, &height, &color1_val,
                   &color2_val);
      self_obj->GradientFillRect(
          x, y, width, height,
          GetObject<lime::Color>(mrb, color1_val, kColorDataType),
          GetObject<lime::Color>(mrb, color2_val, kColorDataType));
    } else if (argc == 7) {
      // GradientFillRect(x, y, width, height, color1, color2, vertical)
      mrb_int x, y, width, height;
      mrb_value color1_val, color2_val;
      mrb_bool vertical;
      mrb_get_args(mrb, "iiiioob", &x, &y, &width, &height, &color1_val,
                   &color2_val, &vertical);
      self_obj->GradientFillRect(
          x, y, width, height,
          GetObject<lime::Color>(mrb, color1_val, kColorDataType),
          GetObject<lime::Color>(mrb, color2_val, kColorDataType), vertical);
    } else {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
    }
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_Clear) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  EXC_BEGIN {
    self_obj->Clear();
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_ClearRect) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);

  mrb_int argc = mrb_get_argc(mrb);

  EXC_BEGIN {
    if (argc == 4) {
      // ClearRect(x, y, width, height)
      mrb_int x, y, width, height;
      mrb_get_args(mrb, "iiii", &x, &y, &width, &height);
      self_obj->ClearRect(x, y, width, height);
    } else if (argc == 1) {
      // ClearRect(rect)
      mrb_value rect_val;
      mrb_get_args(mrb, "o", &rect_val);
      self_obj->ClearRect(GetObject<lime::Rect>(mrb, rect_val, kRectDataType));
    } else {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
    }
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_GetPixel) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
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
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
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

MRB_FUNC(Bitmap_HueChange) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_int hue;
  mrb_get_args(mrb, "i", &hue);

  EXC_BEGIN {
    self_obj->HueChange(hue);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_Blur) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  EXC_BEGIN {
    self_obj->Blur();
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_RadialBlur) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_int angle, division;
  mrb_get_args(mrb, "ii", &angle, &division);

  EXC_BEGIN {
    self_obj->RadialBlur(angle, division);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawText) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);

  EXC_BEGIN {
    mrb_int argc = mrb_get_argc(mrb);

    if (argc < 4) {
      mrb_value rect_obj;
      mrb_value str;
      mrb_int align = 0;
      mrb_get_args(mrb, "oo|i", &rect_obj, &str, &align);

      mrb_value str_raw = mrb_obj_as_string(mrb, str);
      std::string str_std(RSTRING_PTR(str_raw), RSTRING_LEN(str_raw));
      self_obj->DrawText(GetObject<lime::Rect>(mrb, rect_obj, kRectDataType),
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
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
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
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  const char* filename;
  mrb_get_args(mrb, "z", &filename);

  EXC_BEGIN {
    self_obj->SaveFile(filename);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_MaskBlt) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_value dst_rect_val, src_bitmap_val, src_rect_val, mask_val;
  mrb_get_args(mrb, "oooo", &dst_rect_val, &src_bitmap_val, &src_rect_val,
               &mask_val);

  auto dst_rect = GetObject<lime::Rect>(mrb, dst_rect_val, kRectDataType);
  auto src_bitmap =
      GetObject<lime::Bitmap>(mrb, src_bitmap_val, kBitmapDataType);
  auto src_rect = GetObject<lime::Rect>(mrb, src_rect_val, kRectDataType);
  auto mask = GetObject<lime::Bitmap>(mrb, mask_val, kBitmapDataType);

  EXC_BEGIN {
    self_obj->MaskBlt(dst_rect, src_bitmap, src_rect, mask);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

// Attribute: font (RefPtr<Font>)
BINDING_ATTR_OBJECT_REF(Bitmap, lime::Bitmap, Font, lime::Font, kFontDataType);

// Inherited from Dispoable
BINDING_INHERITED_DISPOABLE(Bitmap, lime::Bitmap);

void InitBitmapBinding(mrb_state* mrb) {
  auto klass = DefineClass(mrb, "Bitmap");

  mrb_define_method(mrb, klass, "initialize", Bitmap_initialize,
                    MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "initialize_copy", Bitmap_initialize_copy,
                    MRB_ARGS_REQ(1));
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
  mrb_define_method(mrb, klass, "mask_blt", Bitmap_MaskBlt, MRB_ARGS_REQ(4));
  // Attribute: font
  mrb_define_method(mrb, klass, "font", Bitmap_Font, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "font=", Bitmap_FontEqual, MRB_ARGS_REQ(1));
  // Inherited from Dispoable
  mrb_define_method(mrb, klass, "dispose", Bitmap_Dispose, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "disposed?", Bitmap_IsDisposed,
                    MRB_ARGS_NONE());
}

}  // namespace binding
