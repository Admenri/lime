#include "binding_color.h"

#include "src/utility.h"

namespace binding {

// Define mrb data type
MRB_DATATYPE_DEFINE(Color);

MRB_FUNC(Color_initialize) {
  mrb_int argc = mrb_get_argc(mrb);

  lime::RefPtr<lime::Color> obj = nullptr;
  EXC_BEGIN {
    if (argc == 0) {
      // Color.new
      obj = lime::MakeRefCounted<lime::Color>();
    } else if (argc == 3) {
      // Color.new(red, green, blue)  (alpha = 255)
      mrb_float r, g, b;
      mrb_get_args(mrb, "fff", &r, &g, &b);
      obj = lime::MakeRefCounted<lime::Color>(
          static_cast<float>(r), static_cast<float>(g), static_cast<float>(b));
    } else if (argc == 4) {
      // Color.new(red, green, blue, alpha)
      mrb_float r, g, b, a;
      mrb_get_args(mrb, "ffff", &r, &g, &b, &a);
      obj = lime::MakeRefCounted<lime::Color>(
          static_cast<float>(r), static_cast<float>(g), static_cast<float>(b),
          static_cast<float>(a));
    } else {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
    }
  }
  EXC_END(mrb);

  return SetupSelfData(self, obj.get(), kColorDataType);
}

MRB_FUNC(Color_initialize_copy) {
  mrb_value other;
  mrb_get_args(mrb, "o", &other);

  lime::RefPtr<lime::Color> obj = nullptr;
  EXC_BEGIN {
    auto other_obj = GetObject<lime::Color>(mrb, other, kColorDataType);
    obj = lime::MakeRefCounted<lime::Color>(other_obj);
  }
  EXC_END(mrb);

  return SetupSelfData(self, obj.get(), kColorDataType);
}

MRB_FUNC(Color_Set) {
  auto* self_obj = GetSelfData<lime::Color>(self);

  mrb_int argc = mrb_get_argc(mrb);

  EXC_BEGIN {
    if (argc == 1) {
      // set(color)
      mrb_value color_val;
      mrb_get_args(mrb, "o", &color_val);
      self_obj->Set(GetObject<lime::Color>(mrb, color_val, kColorDataType));
    } else if (argc == 3) {
      // set(red, green, blue)  (alpha = 255)
      mrb_float r, g, b;
      mrb_get_args(mrb, "fff", &r, &g, &b);
      self_obj->Set(static_cast<float>(r), static_cast<float>(g),
                    static_cast<float>(b));
    } else if (argc == 4) {
      // set(red, green, blue, alpha)
      mrb_float r, g, b, a;
      mrb_get_args(mrb, "ffff", &r, &g, &b, &a);
      self_obj->Set(static_cast<float>(r), static_cast<float>(g),
                    static_cast<float>(b), static_cast<float>(a));
    } else {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
    }
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

#define COLOR_PROP_FLOAT(cap)                                           \
  MRB_FUNC(Color_##cap) {                                               \
    auto* self_obj = GetSelfData<lime::Color>(self);                    \
    return mrb_float_value(mrb, static_cast<mrb_float>(self_obj->cap)); \
  }                                                                     \
  MRB_FUNC(Color_##cap##Equal) {                                        \
    auto* self_obj = GetSelfData<lime::Color>(self);                    \
    mrb_float value;                                                    \
    mrb_get_args(mrb, "f", &value);                                     \
    self_obj->cap = static_cast<float>(value);                          \
    return mrb_nil_value();                                             \
  }

COLOR_PROP_FLOAT(red);
COLOR_PROP_FLOAT(green);
COLOR_PROP_FLOAT(blue);
COLOR_PROP_FLOAT(alpha);

#undef COLOR_PROP_FLOAT

// Marshal serialization (instance method _dump) / deserialization (class
// method _load). Per bindgen.md: classes with MARSHAL_DUMP/MARSHAL_LOAD get
// _dump (method) and _load (class method).
MRB_FUNC(Color__dump) {
  auto* self_obj = GetSelfData<lime::Color>(self);
  mrb_int limit;
  mrb_get_args(mrb, "i", &limit);

  EXC_BEGIN {
    auto result = lime::Color::MarshalDump(lime::RefPtr<lime::Color>(self_obj));
    return mrb_str_new(mrb, result.data(), static_cast<mrb_int>(result.size()));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Color__load) {
  mrb_value data;
  mrb_get_args(mrb, "o", &data);

  lime::RefPtr<lime::Color> obj = nullptr;
  EXC_BEGIN {
    obj = lime::Color::MarshalLoad(MRBStringValue(data));
  }
  EXC_END(mrb);

  return WrapObject(mrb, obj.get(), kColorDataType);
}

void InitColorBinding(mrb_state* mrb) {
  auto klass = DefineClass(mrb, "Color");

  mrb_define_method(mrb, klass, "initialize", Color_initialize, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "initialize_copy", Color_initialize_copy,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "set", Color_Set, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "red", Color_red, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "red=", Color_redEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "green", Color_green, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "green=", Color_greenEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "blue", Color_blue, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "blue=", Color_blueEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "alpha", Color_alpha, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "alpha=", Color_alphaEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "_dump", Color__dump, MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, klass, "_load", Color__load, MRB_ARGS_REQ(1));
}

}  // namespace binding
