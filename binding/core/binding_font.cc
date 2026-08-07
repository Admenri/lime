#include "binding_font.h"

#include "binding_color.h"

#include "src/font.h"
#include "src/utility.h"

namespace binding {

// Define mrb data type
MRB_DATATYPE_DEFINE(Font);

MRB_FUNC(Font_initialize) {
  mrb_int argc = mrb_get_argc(mrb);

  rgssx::RefPtr<rgssx::Font> obj = nullptr;
  EXC_BEGIN {
    if (argc == 0) {
      // Font.new
      obj = rgssx::MakeRefCounted<rgssx::Font>();
    } else if (argc == 1) {
      // Font.new(name)
      mrb_value name_val;
      mrb_get_args(mrb, "o", &name_val);
      obj = rgssx::MakeRefCounted<rgssx::Font>(
          GetStringVector(mrb, name_val));
    } else if (argc == 2) {
      // Font.new(name, size)
      mrb_value name_val;
      mrb_int size;
      mrb_get_args(mrb, "oi", &name_val, &size);
      obj = rgssx::MakeRefCounted<rgssx::Font>(
          GetStringVector(mrb, name_val), static_cast<int>(size));
    } else {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
    }
  }
  EXC_END(mrb);

  return SetupSelfData(self, obj.get(), kFontDataType);
}

MRB_FUNC(Font_Exist) {
  const char* name;
  mrb_get_args(mrb, "z", &name);

  EXC_BEGIN {
    return mrb_bool_value(rgssx::Font::Exist(name));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

// Instance attributes -------------------------------------------------------

MRB_FUNC(Font_Name) {
  auto* self_obj = GetSelfData<rgssx::Font>(self);
  EXC_BEGIN {
    auto result = self_obj->Attr_Name();
    return WrapStringVector(mrb, *result);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Font_NameEqual) {
  auto* self_obj = GetSelfData<rgssx::Font>(self);
  mrb_value val;
  mrb_get_args(mrb, "o", &val);

  std::vector<std::string> names = GetStringVector(mrb, val);
  EXC_BEGIN {
    self_obj->Attr_Name(names);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Font_Size) {
  auto* self_obj = GetSelfData<rgssx::Font>(self);
  EXC_BEGIN {
    auto result = self_obj->Attr_Size();
    return mrb_fixnum_value(*result);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Font_SizeEqual) {
  auto* self_obj = GetSelfData<rgssx::Font>(self);
  mrb_int size;
  mrb_get_args(mrb, "i", &size);

  EXC_BEGIN {
    self_obj->Attr_Size(static_cast<int>(size));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

#define FONT_BOOL_ATTR(name, cap)                    \
  MRB_FUNC(Font_##cap) {                             \
    auto* self_obj = GetSelfData<rgssx::Font>(self); \
    EXC_BEGIN {                                      \
      auto result = self_obj->Attr_##cap();          \
      return mrb_bool_value(*result);                \
    }                                                \
    EXC_END(mrb);                                    \
    return mrb_nil_value();                          \
  }                                                  \
  MRB_FUNC(Font_##cap##Equal) {                      \
    auto* self_obj = GetSelfData<rgssx::Font>(self); \
    mrb_bool value;                                  \
    mrb_get_args(mrb, "b", &value);                  \
    EXC_BEGIN {                                      \
      self_obj->Attr_##cap(value);                   \
    }                                                \
    EXC_END(mrb);                                    \
    return mrb_nil_value();                          \
  }

FONT_BOOL_ATTR(name, Bold);
FONT_BOOL_ATTR(name, Italic);
FONT_BOOL_ATTR(name, Outline);
FONT_BOOL_ATTR(name, Shadow);

#undef FONT_BOOL_ATTR

#define FONT_COLOR_ATTR(cap)                                        \
  MRB_FUNC(Font_##cap) {                                            \
    auto* self_obj = GetSelfData<rgssx::Font>(self);                \
    EXC_BEGIN {                                                     \
      auto result = self_obj->Attr_##cap();                         \
      return WrapObject(mrb, result->get(), kColorDataType);        \
    }                                                               \
    EXC_END(mrb);                                                   \
    return mrb_nil_value();                                         \
  }                                                                 \
  MRB_FUNC(Font_##cap##Equal) {                                     \
    auto* self_obj = GetSelfData<rgssx::Font>(self);                \
    mrb_value val;                                                  \
    mrb_get_args(mrb, "o", &val);                                   \
    auto color = GetObject<rgssx::Color>(mrb, val, kColorDataType); \
    EXC_BEGIN {                                                     \
      self_obj->Attr_##cap(color);                                  \
    }                                                               \
    EXC_END(mrb);                                                   \
    return mrb_nil_value();                                         \
  }

FONT_COLOR_ATTR(Color);
FONT_COLOR_ATTR(OutColor);

#undef FONT_COLOR_ATTR

// Class attributes (static) ------------------------------------------------

MRB_FUNC(Font_DefaultName) {
  EXC_BEGIN {
    auto result = rgssx::Font::Attr_DefaultName();
    return WrapStringVector(mrb, *result);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Font_DefaultNameEqual) {
  mrb_value val;
  mrb_get_args(mrb, "o", &val);

  std::vector<std::string> names = GetStringVector(mrb, val);
  EXC_BEGIN {
    rgssx::Font::Attr_DefaultName(names);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Font_DefaultSize) {
  EXC_BEGIN {
    auto result = rgssx::Font::Attr_DefaultSize();
    return mrb_fixnum_value(*result);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Font_DefaultSizeEqual) {
  mrb_int size;
  mrb_get_args(mrb, "i", &size);

  EXC_BEGIN {
    rgssx::Font::Attr_DefaultSize(static_cast<int>(size));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

#define FONT_STATIC_BOOL_ATTR(cap)                    \
  MRB_FUNC(Font_Default##cap) {                       \
    EXC_BEGIN {                                       \
      auto result = rgssx::Font::Attr_Default##cap(); \
      return mrb_bool_value(*result);                 \
    }                                                 \
    EXC_END(mrb);                                     \
    return mrb_nil_value();                           \
  }                                                   \
  MRB_FUNC(Font_Default##cap##Equal) {                \
    mrb_bool value;                                   \
    mrb_get_args(mrb, "b", &value);                   \
    EXC_BEGIN {                                       \
      rgssx::Font::Attr_Default##cap(value);          \
    }                                                 \
    EXC_END(mrb);                                     \
    return mrb_nil_value();                           \
  }

FONT_STATIC_BOOL_ATTR(Bold);
FONT_STATIC_BOOL_ATTR(Italic);
FONT_STATIC_BOOL_ATTR(Outline);
FONT_STATIC_BOOL_ATTR(Shadow);

#undef FONT_STATIC_BOOL_ATTR

#define FONT_STATIC_COLOR_ATTR(cap)                                 \
  MRB_FUNC(Font_Default##cap) {                                     \
    EXC_BEGIN {                                                     \
      auto result = rgssx::Font::Attr_Default##cap();               \
      return WrapObject(mrb, result->get(), kColorDataType);        \
    }                                                               \
    EXC_END(mrb);                                                   \
    return mrb_nil_value();                                         \
  }                                                                 \
  MRB_FUNC(Font_Default##cap##Equal) {                              \
    mrb_value val;                                                  \
    mrb_get_args(mrb, "o", &val);                                   \
    auto color = GetObject<rgssx::Color>(mrb, val, kColorDataType); \
    EXC_BEGIN {                                                     \
      rgssx::Font::Attr_Default##cap(color);                        \
    }                                                               \
    EXC_END(mrb);                                                   \
    return mrb_nil_value();                                         \
  }

FONT_STATIC_COLOR_ATTR(Color);
FONT_STATIC_COLOR_ATTR(OutColor);

#undef FONT_STATIC_COLOR_ATTR

void InitFontBinding(mrb_state* mrb) {
  auto klass = DefineClass(mrb, "Font");

  mrb_define_method(mrb, klass, "initialize", Font_initialize, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "name", Font_Name, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "name=", Font_NameEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "size", Font_Size, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "size=", Font_SizeEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "bold", Font_Bold, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "bold=", Font_BoldEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "italic", Font_Italic, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "italic=", Font_ItalicEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "outline", Font_Outline, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "outline=", Font_OutlineEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "shadow", Font_Shadow, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "shadow=", Font_ShadowEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "color", Font_Color, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "color=", Font_ColorEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "out_color", Font_OutColor, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "out_color=", Font_OutColorEqual,
                    MRB_ARGS_REQ(1));

  mrb_define_class_method(mrb, klass, "exist", Font_Exist, MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, klass, "default_name", Font_DefaultName,
                          MRB_ARGS_NONE());
  mrb_define_class_method(mrb, klass, "default_name=", Font_DefaultNameEqual,
                          MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, klass, "default_size", Font_DefaultSize,
                          MRB_ARGS_NONE());
  mrb_define_class_method(mrb, klass, "default_size=", Font_DefaultSizeEqual,
                          MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, klass, "default_bold", Font_DefaultBold,
                          MRB_ARGS_NONE());
  mrb_define_class_method(mrb, klass, "default_bold=", Font_DefaultBoldEqual,
                          MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, klass, "default_italic", Font_DefaultItalic,
                          MRB_ARGS_NONE());
  mrb_define_class_method(
      mrb, klass, "default_italic=", Font_DefaultItalicEqual, MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, klass, "default_outline", Font_DefaultOutline,
                          MRB_ARGS_NONE());
  mrb_define_class_method(mrb, klass,
                          "default_outline=", Font_DefaultOutlineEqual,
                          MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, klass, "default_shadow", Font_DefaultShadow,
                          MRB_ARGS_NONE());
  mrb_define_class_method(
      mrb, klass, "default_shadow=", Font_DefaultShadowEqual, MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, klass, "default_color", Font_DefaultColor,
                          MRB_ARGS_NONE());
  mrb_define_class_method(mrb, klass, "default_color=", Font_DefaultColorEqual,
                          MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, klass, "default_out_color", Font_DefaultOutColor,
                          MRB_ARGS_NONE());
  mrb_define_class_method(mrb, klass,
                          "default_out_color=", Font_DefaultOutColorEqual,
                          MRB_ARGS_REQ(1));
}

}  // namespace binding
