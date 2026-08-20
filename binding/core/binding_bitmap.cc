#include "binding_bitmap.h"

#include "binding_color.h"
#include "binding_font.h"
#include "binding_palette.h"
#include "binding_rect.h"
#include "binding_vector.h"

#include "src/bitmap.h"
#include "src/utility.h"

namespace binding {

// Define mrb data type
MRB_DATATYPE_DEFINE(Bitmap);

namespace {

// Ruby Array of Vector2 (or nil) <-> std::vector<RefPtr<Vector2>>. Any other
// type raises a TypeError.
std::vector<lime::RefPtr<lime::Vector2>> GetVector2Array(mrb_state* mrb,
                                                         mrb_value val) {
  std::vector<lime::RefPtr<lime::Vector2>> result;
  if (mrb_nil_p(val))
    return result;

  if (!mrb_array_p(val))
    mrb_raise(mrb, E_TYPE_ERROR, "expected Array of Vector2");

  mrb_int len = RARRAY_LEN(val);
  mrb_value* ptr = RARRAY_PTR(val);
  result.reserve(static_cast<size_t>(len));
  for (mrb_int i = 0; i < len; ++i)
    result.push_back(GetObject<lime::Vector2>(mrb, ptr[i], kVector2DataType));
  return result;
}

}  // namespace

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
    return mrb_fixnum_value(self_obj->GetWidth());
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_Height) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  EXC_BEGIN {
    return mrb_fixnum_value(self_obj->GetHeight());
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

MRB_FUNC(Bitmap_ToPalette) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  EXC_BEGIN {
    auto result = self_obj->ToPalette();
    return WrapObject(mrb, result.get(), kPaletteDataType);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_UpdateWithPalette) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_value palette_val;
  mrb_get_args(mrb, "o", &palette_val);

  auto palette = GetObject<lime::Palette>(mrb, palette_val, kPaletteDataType);

  EXC_BEGIN {
    self_obj->UpdateWithPalette(palette);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_SetFilter) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_int value;
  mrb_get_args(mrb, "i", &value);

  EXC_BEGIN {
    self_obj->SetFilter(value);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_SetWrap) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_int value;
  mrb_get_args(mrb, "i", &value);

  EXC_BEGIN {
    self_obj->SetWrap(value);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

// ---------------------------------------------------------------------------
// Basic shapes drawing functions (raylib passthrough)
// ---------------------------------------------------------------------------

MRB_FUNC(Bitmap_DrawPixel) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_int x, y;
  mrb_value color_val;
  mrb_get_args(mrb, "iio", &x, &y, &color_val);
  auto color = GetObject<lime::Color>(mrb, color_val, kColorDataType);

  EXC_BEGIN {
    self_obj->DrawPixel(x, y, color);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawLine) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_int argc = mrb_get_argc(mrb);

  EXC_BEGIN {
    if (argc == 5) {
      // DrawLine(x1, y1, x2, y2, color)
      mrb_int x1, y1, x2, y2;
      mrb_value color_val;
      mrb_get_args(mrb, "iiiio", &x1, &y1, &x2, &y2, &color_val);
      self_obj->DrawLine(
          x1, y1, x2, y2,
          GetObject<lime::Color>(mrb, color_val, kColorDataType));
    } else if (argc == 3) {
      // DrawLine(start_pos, end_pos, color)
      mrb_value start_val, end_val, color_val;
      mrb_get_args(mrb, "ooo", &start_val, &end_val, &color_val);
      self_obj->DrawLine(
          GetObject<lime::Vector2>(mrb, start_val, kVector2DataType),
          GetObject<lime::Vector2>(mrb, end_val, kVector2DataType),
          GetObject<lime::Color>(mrb, color_val, kColorDataType));
    } else {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
    }
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawLineEx) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_value start_val, end_val, color_val;
  mrb_float thick;
  mrb_get_args(mrb, "oofo", &start_val, &end_val, &thick, &color_val);

  EXC_BEGIN {
    self_obj->DrawLineEx(
        GetObject<lime::Vector2>(mrb, start_val, kVector2DataType),
        GetObject<lime::Vector2>(mrb, end_val, kVector2DataType), (float)thick,
        GetObject<lime::Color>(mrb, color_val, kColorDataType));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawLineStrip) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_value points_val, color_val;
  mrb_get_args(mrb, "oo", &points_val, &color_val);

  EXC_BEGIN {
    self_obj->DrawLineStrip(
        GetVector2Array(mrb, points_val),
        GetObject<lime::Color>(mrb, color_val, kColorDataType));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawLineBezier) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_value start_val, end_val, color_val;
  mrb_float thick;
  mrb_get_args(mrb, "oofo", &start_val, &end_val, &thick, &color_val);

  EXC_BEGIN {
    self_obj->DrawLineBezier(
        GetObject<lime::Vector2>(mrb, start_val, kVector2DataType),
        GetObject<lime::Vector2>(mrb, end_val, kVector2DataType), (float)thick,
        GetObject<lime::Color>(mrb, color_val, kColorDataType));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawLineDashed) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_value start_val, end_val, color_val;
  mrb_int dash_size, space_size;
  mrb_get_args(mrb, "ooiio", &start_val, &end_val, &dash_size, &space_size,
               &color_val);

  EXC_BEGIN {
    self_obj->DrawLineDashed(
        GetObject<lime::Vector2>(mrb, start_val, kVector2DataType),
        GetObject<lime::Vector2>(mrb, end_val, kVector2DataType), dash_size,
        space_size, GetObject<lime::Color>(mrb, color_val, kColorDataType));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawTriangle) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_value v1_val, v2_val, v3_val, color_val;
  mrb_get_args(mrb, "oooo", &v1_val, &v2_val, &v3_val, &color_val);

  EXC_BEGIN {
    self_obj->DrawTriangle(
        GetObject<lime::Vector2>(mrb, v1_val, kVector2DataType),
        GetObject<lime::Vector2>(mrb, v2_val, kVector2DataType),
        GetObject<lime::Vector2>(mrb, v3_val, kVector2DataType),
        GetObject<lime::Color>(mrb, color_val, kColorDataType));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawTriangleGradient) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_value v1_val, v2_val, v3_val, c1_val, c2_val, c3_val;
  mrb_get_args(mrb, "oooooo", &v1_val, &v2_val, &v3_val, &c1_val, &c2_val,
               &c3_val);

  EXC_BEGIN {
    self_obj->DrawTriangleGradient(
        GetObject<lime::Vector2>(mrb, v1_val, kVector2DataType),
        GetObject<lime::Vector2>(mrb, v2_val, kVector2DataType),
        GetObject<lime::Vector2>(mrb, v3_val, kVector2DataType),
        GetObject<lime::Color>(mrb, c1_val, kColorDataType),
        GetObject<lime::Color>(mrb, c2_val, kColorDataType),
        GetObject<lime::Color>(mrb, c3_val, kColorDataType));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawTriangleLines) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_value v1_val, v2_val, v3_val, color_val;
  mrb_get_args(mrb, "oooo", &v1_val, &v2_val, &v3_val, &color_val);

  EXC_BEGIN {
    self_obj->DrawTriangleLines(
        GetObject<lime::Vector2>(mrb, v1_val, kVector2DataType),
        GetObject<lime::Vector2>(mrb, v2_val, kVector2DataType),
        GetObject<lime::Vector2>(mrb, v3_val, kVector2DataType),
        GetObject<lime::Color>(mrb, color_val, kColorDataType));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawTriangleFan) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_value points_val, color_val;
  mrb_get_args(mrb, "oo", &points_val, &color_val);

  EXC_BEGIN {
    self_obj->DrawTriangleFan(
        GetVector2Array(mrb, points_val),
        GetObject<lime::Color>(mrb, color_val, kColorDataType));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawTriangleStrip) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_value points_val, color_val;
  mrb_get_args(mrb, "oo", &points_val, &color_val);

  EXC_BEGIN {
    self_obj->DrawTriangleStrip(
        GetVector2Array(mrb, points_val),
        GetObject<lime::Color>(mrb, color_val, kColorDataType));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawRectangle) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_int argc = mrb_get_argc(mrb);

  EXC_BEGIN {
    if (argc == 5) {
      // DrawRectangle(x, y, width, height, color)
      mrb_int x, y, width, height;
      mrb_value color_val;
      mrb_get_args(mrb, "iiiio", &x, &y, &width, &height, &color_val);
      self_obj->DrawRectangle(
          x, y, width, height,
          GetObject<lime::Color>(mrb, color_val, kColorDataType));
    } else if (argc == 3) {
      // DrawRectangle(position, size, color)
      mrb_value position_val, size_val, color_val;
      mrb_get_args(mrb, "ooo", &position_val, &size_val, &color_val);
      self_obj->DrawRectangle(
          GetObject<lime::Vector2>(mrb, position_val, kVector2DataType),
          GetObject<lime::Vector2>(mrb, size_val, kVector2DataType),
          GetObject<lime::Color>(mrb, color_val, kColorDataType));
    } else if (argc == 2) {
      // DrawRectangle(rect, color)
      mrb_value rect_val, color_val;
      mrb_get_args(mrb, "oo", &rect_val, &color_val);
      self_obj->DrawRectangle(
          GetObject<lime::Rect>(mrb, rect_val, kRectDataType),
          GetObject<lime::Color>(mrb, color_val, kColorDataType));
    } else {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
    }
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawRectanglePro) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_value rect_val, origin_val, color_val;
  mrb_float rotation;
  mrb_get_args(mrb, "oofo", &rect_val, &origin_val, &rotation, &color_val);

  EXC_BEGIN {
    self_obj->DrawRectanglePro(
        GetObject<lime::Rect>(mrb, rect_val, kRectDataType),
        GetObject<lime::Vector2>(mrb, origin_val, kVector2DataType),
        (float)rotation,
        GetObject<lime::Color>(mrb, color_val, kColorDataType));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawRectangleGradientV) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_int x, y, width, height;
  mrb_value top_val, bottom_val;
  mrb_get_args(mrb, "iiiioo", &x, &y, &width, &height, &top_val, &bottom_val);

  EXC_BEGIN {
    self_obj->DrawRectangleGradientV(
        x, y, width, height,
        GetObject<lime::Color>(mrb, top_val, kColorDataType),
        GetObject<lime::Color>(mrb, bottom_val, kColorDataType));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawRectangleGradientH) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_int x, y, width, height;
  mrb_value left_val, right_val;
  mrb_get_args(mrb, "iiiioo", &x, &y, &width, &height, &left_val, &right_val);

  EXC_BEGIN {
    self_obj->DrawRectangleGradientH(
        x, y, width, height,
        GetObject<lime::Color>(mrb, left_val, kColorDataType),
        GetObject<lime::Color>(mrb, right_val, kColorDataType));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawRectangleGradientEx) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_value rect_val, c1_val, c2_val, c3_val, c4_val;
  mrb_get_args(mrb, "ooooo", &rect_val, &c1_val, &c2_val, &c3_val, &c4_val);

  EXC_BEGIN {
    self_obj->DrawRectangleGradientEx(
        GetObject<lime::Rect>(mrb, rect_val, kRectDataType),
        GetObject<lime::Color>(mrb, c1_val, kColorDataType),
        GetObject<lime::Color>(mrb, c2_val, kColorDataType),
        GetObject<lime::Color>(mrb, c3_val, kColorDataType),
        GetObject<lime::Color>(mrb, c4_val, kColorDataType));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawRectangleLines) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_int x, y, width, height;
  mrb_value color_val;
  mrb_get_args(mrb, "iiiio", &x, &y, &width, &height, &color_val);

  EXC_BEGIN {
    self_obj->DrawRectangleLines(
        x, y, width, height,
        GetObject<lime::Color>(mrb, color_val, kColorDataType));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawRectangleLinesEx) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_value rect_val, color_val;
  mrb_float thick;
  mrb_get_args(mrb, "ofo", &rect_val, &thick, &color_val);

  EXC_BEGIN {
    self_obj->DrawRectangleLinesEx(
        GetObject<lime::Rect>(mrb, rect_val, kRectDataType), (float)thick,
        GetObject<lime::Color>(mrb, color_val, kColorDataType));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawRectangleRounded) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_value rect_val, color_val;
  mrb_float roundness;
  mrb_int segments;
  mrb_get_args(mrb, "ofio", &rect_val, &roundness, &segments, &color_val);

  EXC_BEGIN {
    self_obj->DrawRectangleRounded(
        GetObject<lime::Rect>(mrb, rect_val, kRectDataType), (float)roundness,
        segments, GetObject<lime::Color>(mrb, color_val, kColorDataType));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawRectangleRoundedLines) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_value rect_val, color_val;
  mrb_float roundness;
  mrb_int segments;
  mrb_get_args(mrb, "ofio", &rect_val, &roundness, &segments, &color_val);

  EXC_BEGIN {
    self_obj->DrawRectangleRoundedLines(
        GetObject<lime::Rect>(mrb, rect_val, kRectDataType), (float)roundness,
        segments, GetObject<lime::Color>(mrb, color_val, kColorDataType));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawRectangleRoundedLinesEx) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_value rect_val, color_val;
  mrb_float roundness, thick;
  mrb_int segments;
  mrb_get_args(mrb, "ofifo", &rect_val, &roundness, &segments, &thick,
               &color_val);

  EXC_BEGIN {
    self_obj->DrawRectangleRoundedLinesEx(
        GetObject<lime::Rect>(mrb, rect_val, kRectDataType), (float)roundness,
        segments, (float)thick,
        GetObject<lime::Color>(mrb, color_val, kColorDataType));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawPoly) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_value center_val, color_val;
  mrb_int sides;
  mrb_float radius, rotation;
  mrb_get_args(mrb, "oiffo", &center_val, &sides, &radius, &rotation,
               &color_val);

  EXC_BEGIN {
    self_obj->DrawPoly(
        GetObject<lime::Vector2>(mrb, center_val, kVector2DataType), sides,
        (float)radius, (float)rotation,
        GetObject<lime::Color>(mrb, color_val, kColorDataType));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawPolyLines) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_value center_val, color_val;
  mrb_int sides;
  mrb_float radius, rotation;
  mrb_get_args(mrb, "oiffo", &center_val, &sides, &radius, &rotation,
               &color_val);

  EXC_BEGIN {
    self_obj->DrawPolyLines(
        GetObject<lime::Vector2>(mrb, center_val, kVector2DataType), sides,
        (float)radius, (float)rotation,
        GetObject<lime::Color>(mrb, color_val, kColorDataType));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawPolyLinesEx) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_value center_val, color_val;
  mrb_int sides;
  mrb_float radius, rotation, thick;
  mrb_get_args(mrb, "oifffo", &center_val, &sides, &radius, &rotation, &thick,
               &color_val);

  EXC_BEGIN {
    self_obj->DrawPolyLinesEx(
        GetObject<lime::Vector2>(mrb, center_val, kVector2DataType), sides,
        (float)radius, (float)rotation, (float)thick,
        GetObject<lime::Color>(mrb, color_val, kColorDataType));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawCircle) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_int argc = mrb_get_argc(mrb);

  EXC_BEGIN {
    if (argc == 4) {
      // DrawCircle(x, y, radius, color)
      mrb_int x, y;
      mrb_float radius;
      mrb_value color_val;
      mrb_get_args(mrb, "iifo", &x, &y, &radius, &color_val);
      self_obj->DrawCircle(
          x, y, (float)radius,
          GetObject<lime::Color>(mrb, color_val, kColorDataType));
    } else if (argc == 3) {
      // DrawCircle(center, radius, color)
      mrb_value center_val, color_val;
      mrb_float radius;
      mrb_get_args(mrb, "ofo", &center_val, &radius, &color_val);
      self_obj->DrawCircle(
          GetObject<lime::Vector2>(mrb, center_val, kVector2DataType),
          (float)radius,
          GetObject<lime::Color>(mrb, color_val, kColorDataType));
    } else {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
    }
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawCircleGradient) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_value center_val, inner_val, outer_val;
  mrb_float radius;
  mrb_get_args(mrb, "ofoo", &center_val, &radius, &inner_val, &outer_val);

  EXC_BEGIN {
    self_obj->DrawCircleGradient(
        GetObject<lime::Vector2>(mrb, center_val, kVector2DataType),
        (float)radius, GetObject<lime::Color>(mrb, inner_val, kColorDataType),
        GetObject<lime::Color>(mrb, outer_val, kColorDataType));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawCircleSector) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_value center_val, color_val;
  mrb_float radius, start_angle, end_angle;
  mrb_int segments;
  mrb_get_args(mrb, "offfio", &center_val, &radius, &start_angle, &end_angle,
               &segments, &color_val);

  EXC_BEGIN {
    self_obj->DrawCircleSector(
        GetObject<lime::Vector2>(mrb, center_val, kVector2DataType),
        (float)radius, (float)start_angle, (float)end_angle, segments,
        GetObject<lime::Color>(mrb, color_val, kColorDataType));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawCircleSectorLines) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_value center_val, color_val;
  mrb_float radius, start_angle, end_angle;
  mrb_int segments;
  mrb_get_args(mrb, "offfio", &center_val, &radius, &start_angle, &end_angle,
               &segments, &color_val);

  EXC_BEGIN {
    self_obj->DrawCircleSectorLines(
        GetObject<lime::Vector2>(mrb, center_val, kVector2DataType),
        (float)radius, (float)start_angle, (float)end_angle, segments,
        GetObject<lime::Color>(mrb, color_val, kColorDataType));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawCircleLines) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_int argc = mrb_get_argc(mrb);

  EXC_BEGIN {
    if (argc == 4) {
      // DrawCircleLines(x, y, radius, color)
      mrb_int x, y;
      mrb_float radius;
      mrb_value color_val;
      mrb_get_args(mrb, "iifo", &x, &y, &radius, &color_val);
      self_obj->DrawCircleLines(
          x, y, (float)radius,
          GetObject<lime::Color>(mrb, color_val, kColorDataType));
    } else if (argc == 3) {
      // DrawCircleLines(center, radius, color)
      mrb_value center_val, color_val;
      mrb_float radius;
      mrb_get_args(mrb, "ofo", &center_val, &radius, &color_val);
      self_obj->DrawCircleLines(
          GetObject<lime::Vector2>(mrb, center_val, kVector2DataType),
          (float)radius,
          GetObject<lime::Color>(mrb, color_val, kColorDataType));
    } else {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
    }
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawCircleLinesEx) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_value center_val, color_val;
  mrb_float radius, thick;
  mrb_get_args(mrb, "offo", &center_val, &radius, &thick, &color_val);

  EXC_BEGIN {
    self_obj->DrawCircleLinesEx(
        GetObject<lime::Vector2>(mrb, center_val, kVector2DataType),
        (float)radius, (float)thick,
        GetObject<lime::Color>(mrb, color_val, kColorDataType));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawEllipse) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_int argc = mrb_get_argc(mrb);

  EXC_BEGIN {
    if (argc == 5) {
      // DrawEllipse(x, y, radius_h, radius_v, color)
      mrb_int x, y;
      mrb_float radius_h, radius_v;
      mrb_value color_val;
      mrb_get_args(mrb, "iiffo", &x, &y, &radius_h, &radius_v, &color_val);
      self_obj->DrawEllipse(
          x, y, (float)radius_h, (float)radius_v,
          GetObject<lime::Color>(mrb, color_val, kColorDataType));
    } else if (argc == 4) {
      // DrawEllipse(center, radius_h, radius_v, color)
      mrb_value center_val, color_val;
      mrb_float radius_h, radius_v;
      mrb_get_args(mrb, "offo", &center_val, &radius_h, &radius_v, &color_val);
      self_obj->DrawEllipse(
          GetObject<lime::Vector2>(mrb, center_val, kVector2DataType),
          (float)radius_h, (float)radius_v,
          GetObject<lime::Color>(mrb, color_val, kColorDataType));
    } else {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
    }
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawEllipseLines) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_int argc = mrb_get_argc(mrb);

  EXC_BEGIN {
    if (argc == 5) {
      // DrawEllipseLines(x, y, radius_h, radius_v, color)
      mrb_int x, y;
      mrb_float radius_h, radius_v;
      mrb_value color_val;
      mrb_get_args(mrb, "iiffo", &x, &y, &radius_h, &radius_v, &color_val);
      self_obj->DrawEllipseLines(
          x, y, (float)radius_h, (float)radius_v,
          GetObject<lime::Color>(mrb, color_val, kColorDataType));
    } else if (argc == 4) {
      // DrawEllipseLines(center, radius_h, radius_v, color)
      mrb_value center_val, color_val;
      mrb_float radius_h, radius_v;
      mrb_get_args(mrb, "offo", &center_val, &radius_h, &radius_v, &color_val);
      self_obj->DrawEllipseLines(
          GetObject<lime::Vector2>(mrb, center_val, kVector2DataType),
          (float)radius_h, (float)radius_v,
          GetObject<lime::Color>(mrb, color_val, kColorDataType));
    } else {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
    }
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawRing) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_value center_val, color_val;
  mrb_float inner_radius, outer_radius, start_angle, end_angle;
  mrb_int segments;
  mrb_get_args(mrb, "offffio", &center_val, &inner_radius, &outer_radius,
               &start_angle, &end_angle, &segments, &color_val);

  EXC_BEGIN {
    self_obj->DrawRing(
        GetObject<lime::Vector2>(mrb, center_val, kVector2DataType),
        (float)inner_radius, (float)outer_radius, (float)start_angle,
        (float)end_angle, segments,
        GetObject<lime::Color>(mrb, color_val, kColorDataType));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawRingLines) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_value center_val, color_val;
  mrb_float inner_radius, outer_radius, start_angle, end_angle;
  mrb_int segments;
  mrb_get_args(mrb, "offffio", &center_val, &inner_radius, &outer_radius,
               &start_angle, &end_angle, &segments, &color_val);

  EXC_BEGIN {
    self_obj->DrawRingLines(
        GetObject<lime::Vector2>(mrb, center_val, kVector2DataType),
        (float)inner_radius, (float)outer_radius, (float)start_angle,
        (float)end_angle, segments,
        GetObject<lime::Color>(mrb, color_val, kColorDataType));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

// ---------------------------------------------------------------------------
// Splines drawing functions (raylib passthrough)
// ---------------------------------------------------------------------------

MRB_FUNC(Bitmap_DrawSplineLinear) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_value points_val, color_val;
  mrb_float thick;
  mrb_get_args(mrb, "ofo", &points_val, &thick, &color_val);

  EXC_BEGIN {
    self_obj->DrawSplineLinear(
        GetVector2Array(mrb, points_val), (float)thick,
        GetObject<lime::Color>(mrb, color_val, kColorDataType));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawSplineBasis) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_value points_val, color_val;
  mrb_float thick;
  mrb_get_args(mrb, "ofo", &points_val, &thick, &color_val);

  EXC_BEGIN {
    self_obj->DrawSplineBasis(
        GetVector2Array(mrb, points_val), (float)thick,
        GetObject<lime::Color>(mrb, color_val, kColorDataType));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawSplineCatmullRom) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_value points_val, color_val;
  mrb_float thick;
  mrb_get_args(mrb, "ofo", &points_val, &thick, &color_val);

  EXC_BEGIN {
    self_obj->DrawSplineCatmullRom(
        GetVector2Array(mrb, points_val), (float)thick,
        GetObject<lime::Color>(mrb, color_val, kColorDataType));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawSplineBezierQuadratic) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_value points_val, color_val;
  mrb_float thick;
  mrb_get_args(mrb, "ofo", &points_val, &thick, &color_val);

  EXC_BEGIN {
    self_obj->DrawSplineBezierQuadratic(
        GetVector2Array(mrb, points_val), (float)thick,
        GetObject<lime::Color>(mrb, color_val, kColorDataType));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawSplineBezierCubic) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_value points_val, color_val;
  mrb_float thick;
  mrb_get_args(mrb, "ofo", &points_val, &thick, &color_val);

  EXC_BEGIN {
    self_obj->DrawSplineBezierCubic(
        GetVector2Array(mrb, points_val), (float)thick,
        GetObject<lime::Color>(mrb, color_val, kColorDataType));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawSplineSegmentLinear) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_value p1_val, p2_val, color_val;
  mrb_float thick;
  mrb_get_args(mrb, "oofo", &p1_val, &p2_val, &thick, &color_val);

  EXC_BEGIN {
    self_obj->DrawSplineSegmentLinear(
        GetObject<lime::Vector2>(mrb, p1_val, kVector2DataType),
        GetObject<lime::Vector2>(mrb, p2_val, kVector2DataType), (float)thick,
        GetObject<lime::Color>(mrb, color_val, kColorDataType));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawSplineSegmentBasis) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_value p1_val, p2_val, p3_val, p4_val, color_val;
  mrb_float thick;
  mrb_get_args(mrb, "oooofo", &p1_val, &p2_val, &p3_val, &p4_val, &thick,
               &color_val);

  EXC_BEGIN {
    self_obj->DrawSplineSegmentBasis(
        GetObject<lime::Vector2>(mrb, p1_val, kVector2DataType),
        GetObject<lime::Vector2>(mrb, p2_val, kVector2DataType),
        GetObject<lime::Vector2>(mrb, p3_val, kVector2DataType),
        GetObject<lime::Vector2>(mrb, p4_val, kVector2DataType), (float)thick,
        GetObject<lime::Color>(mrb, color_val, kColorDataType));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawSplineSegmentCatmullRom) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_value p1_val, p2_val, p3_val, p4_val, color_val;
  mrb_float thick;
  mrb_get_args(mrb, "oooofo", &p1_val, &p2_val, &p3_val, &p4_val, &thick,
               &color_val);

  EXC_BEGIN {
    self_obj->DrawSplineSegmentCatmullRom(
        GetObject<lime::Vector2>(mrb, p1_val, kVector2DataType),
        GetObject<lime::Vector2>(mrb, p2_val, kVector2DataType),
        GetObject<lime::Vector2>(mrb, p3_val, kVector2DataType),
        GetObject<lime::Vector2>(mrb, p4_val, kVector2DataType), (float)thick,
        GetObject<lime::Color>(mrb, color_val, kColorDataType));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawSplineSegmentBezierQuadratic) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_value p1_val, c2_val, p3_val, color_val;
  mrb_float thick;
  mrb_get_args(mrb, "ooofo", &p1_val, &c2_val, &p3_val, &thick, &color_val);

  EXC_BEGIN {
    self_obj->DrawSplineSegmentBezierQuadratic(
        GetObject<lime::Vector2>(mrb, p1_val, kVector2DataType),
        GetObject<lime::Vector2>(mrb, c2_val, kVector2DataType),
        GetObject<lime::Vector2>(mrb, p3_val, kVector2DataType), (float)thick,
        GetObject<lime::Color>(mrb, color_val, kColorDataType));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Bitmap_DrawSplineSegmentBezierCubic) {
  auto* self_obj = GetSelfData<lime::Bitmap>(self);
  mrb_value p1_val, c2_val, c3_val, p4_val, color_val;
  mrb_float thick;
  mrb_get_args(mrb, "oooofo", &p1_val, &c2_val, &c3_val, &p4_val, &thick,
               &color_val);

  EXC_BEGIN {
    self_obj->DrawSplineSegmentBezierCubic(
        GetObject<lime::Vector2>(mrb, p1_val, kVector2DataType),
        GetObject<lime::Vector2>(mrb, c2_val, kVector2DataType),
        GetObject<lime::Vector2>(mrb, c3_val, kVector2DataType),
        GetObject<lime::Vector2>(mrb, p4_val, kVector2DataType), (float)thick,
        GetObject<lime::Color>(mrb, color_val, kColorDataType));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

// Attribute: font (RefPtr<Font>)
BINDING_ATTR_OBJECT_REF(Bitmap, lime::Bitmap, Font, lime::Font, kFontDataType);

// Attribute: shape_bitmap (RefPtr<Bitmap>)
BINDING_ATTR_OBJECT_REF(Bitmap,
                        lime::Bitmap,
                        ShapeBitmap,
                        lime::Bitmap,
                        kBitmapDataType);

// Inherited from Dispoable
BINDING_INHERITED_DISPOABLE(Bitmap, lime::Bitmap);

void InitBitmapBinding(mrb_state* mrb) {
  auto klass = DefineClass(mrb, "Bitmap");

  // Texture filter / wrap constants (values from rlgl.h, the RL_TEXTURE_
  // prefix is removed). Used with Bitmap#set_filter / Bitmap#set_wrap.
  mrb_define_const(mrb, klass, "FILTER_NEAREST",
                   mrb_fixnum_value(RL_TEXTURE_FILTER_NEAREST));
  mrb_define_const(mrb, klass, "FILTER_LINEAR",
                   mrb_fixnum_value(RL_TEXTURE_FILTER_LINEAR));
  mrb_define_const(mrb, klass, "FILTER_MIP_NEAREST",
                   mrb_fixnum_value(RL_TEXTURE_FILTER_MIP_NEAREST));
  mrb_define_const(mrb, klass, "FILTER_NEAREST_MIP_LINEAR",
                   mrb_fixnum_value(RL_TEXTURE_FILTER_NEAREST_MIP_LINEAR));
  mrb_define_const(mrb, klass, "FILTER_LINEAR_MIP_NEAREST",
                   mrb_fixnum_value(RL_TEXTURE_FILTER_LINEAR_MIP_NEAREST));
  mrb_define_const(mrb, klass, "FILTER_MIP_LINEAR",
                   mrb_fixnum_value(RL_TEXTURE_FILTER_MIP_LINEAR));
  mrb_define_const(mrb, klass, "FILTER_ANISOTROPIC",
                   mrb_fixnum_value(RL_TEXTURE_FILTER_ANISOTROPIC));
  mrb_define_const(mrb, klass, "MIPMAP_BIAS_RATIO",
                   mrb_fixnum_value(RL_TEXTURE_MIPMAP_BIAS_RATIO));
  mrb_define_const(mrb, klass, "WRAP_REPEAT",
                   mrb_fixnum_value(RL_TEXTURE_WRAP_REPEAT));
  mrb_define_const(mrb, klass, "WRAP_CLAMP",
                   mrb_fixnum_value(RL_TEXTURE_WRAP_CLAMP));
  mrb_define_const(mrb, klass, "WRAP_MIRROR_REPEAT",
                   mrb_fixnum_value(RL_TEXTURE_WRAP_MIRROR_REPEAT));
  mrb_define_const(mrb, klass, "WRAP_MIRROR_CLAMP",
                   mrb_fixnum_value(RL_TEXTURE_WRAP_MIRROR_CLAMP));

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
  mrb_define_method(mrb, klass, "mask_blt", Bitmap_MaskBlt, MRB_ARGS_REQ(4));
  mrb_define_method(mrb, klass, "to_palette", Bitmap_ToPalette,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "update_with_palette",
                    Bitmap_UpdateWithPalette, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "set_filter", Bitmap_SetFilter,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "set_wrap", Bitmap_SetWrap, MRB_ARGS_REQ(1));
  // Basic shapes drawing functions
  mrb_define_method(mrb, klass, "draw_pixel", Bitmap_DrawPixel, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "draw_line", Bitmap_DrawLine, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "draw_line_ex", Bitmap_DrawLineEx,
                    MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "draw_line_strip", Bitmap_DrawLineStrip,
                    MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "draw_line_bezier", Bitmap_DrawLineBezier,
                    MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "draw_line_dashed", Bitmap_DrawLineDashed,
                    MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "draw_triangle", Bitmap_DrawTriangle,
                    MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "draw_triangle_gradient",
                    Bitmap_DrawTriangleGradient, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "draw_triangle_lines", Bitmap_DrawTriangleLines,
                    MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "draw_triangle_fan", Bitmap_DrawTriangleFan,
                    MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "draw_triangle_strip", Bitmap_DrawTriangleStrip,
                    MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "draw_rectangle", Bitmap_DrawRectangle,
                    MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "draw_rectangle_pro", Bitmap_DrawRectanglePro,
                    MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "draw_rectangle_gradient_v",
                    Bitmap_DrawRectangleGradientV, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "draw_rectangle_gradient_h",
                    Bitmap_DrawRectangleGradientH, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "draw_rectangle_gradient_ex",
                    Bitmap_DrawRectangleGradientEx, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "draw_rectangle_lines",
                    Bitmap_DrawRectangleLines, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "draw_rectangle_lines_ex",
                    Bitmap_DrawRectangleLinesEx, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "draw_rectangle_rounded",
                    Bitmap_DrawRectangleRounded, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "draw_rectangle_rounded_lines",
                    Bitmap_DrawRectangleRoundedLines, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "draw_rectangle_rounded_lines_ex",
                    Bitmap_DrawRectangleRoundedLinesEx, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "draw_poly", Bitmap_DrawPoly, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "draw_poly_lines", Bitmap_DrawPolyLines,
                    MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "draw_poly_lines_ex", Bitmap_DrawPolyLinesEx,
                    MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "draw_circle", Bitmap_DrawCircle,
                    MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "draw_circle_gradient",
                    Bitmap_DrawCircleGradient, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "draw_circle_sector", Bitmap_DrawCircleSector,
                    MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "draw_circle_sector_lines",
                    Bitmap_DrawCircleSectorLines, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "draw_circle_lines", Bitmap_DrawCircleLines,
                    MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "draw_circle_lines_ex",
                    Bitmap_DrawCircleLinesEx, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "draw_ellipse", Bitmap_DrawEllipse,
                    MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "draw_ellipse_lines", Bitmap_DrawEllipseLines,
                    MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "draw_ring", Bitmap_DrawRing, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "draw_ring_lines", Bitmap_DrawRingLines,
                    MRB_ARGS_ANY());
  // Splines drawing functions
  mrb_define_method(mrb, klass, "draw_spline_linear", Bitmap_DrawSplineLinear,
                    MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "draw_spline_basis", Bitmap_DrawSplineBasis,
                    MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "draw_spline_catmull_rom",
                    Bitmap_DrawSplineCatmullRom, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "draw_spline_bezier_quadratic",
                    Bitmap_DrawSplineBezierQuadratic, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "draw_spline_bezier_cubic",
                    Bitmap_DrawSplineBezierCubic, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "draw_spline_segment_linear",
                    Bitmap_DrawSplineSegmentLinear, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "draw_spline_segment_basis",
                    Bitmap_DrawSplineSegmentBasis, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "draw_spline_segment_catmull_rom",
                    Bitmap_DrawSplineSegmentCatmullRom, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "draw_spline_segment_bezier_quadratic",
                    Bitmap_DrawSplineSegmentBezierQuadratic, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "draw_spline_segment_bezier_cubic",
                    Bitmap_DrawSplineSegmentBezierCubic, MRB_ARGS_ANY());
  // Attribute: font
  mrb_define_method(mrb, klass, "font", Bitmap_Font, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "font=", Bitmap_FontEqual, MRB_ARGS_REQ(1));
  // Attribute: shape_bitmap
  mrb_define_method(mrb, klass, "shape_bitmap", Bitmap_ShapeBitmap,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "shape_bitmap=", Bitmap_ShapeBitmapEqual,
                    MRB_ARGS_REQ(1));
  // Inherited from Dispoable
  mrb_define_method(mrb, klass, "dispose", Bitmap_Dispose, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "disposed?", Bitmap_IsDisposed,
                    MRB_ARGS_NONE());
}

}  // namespace binding
