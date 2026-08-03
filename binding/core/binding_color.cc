#include "binding_color.h"

#include "src/utility.h"

namespace binding {

// Define mrb data type
MRB_DATATYPE_DEFINE(Color);

MRB_FUNC(Color_initialize) {
  const mrb_value* args;
  mrb_int argc;
  mrb_get_args(mrb, "*", &args, &argc);

  rgssx::RefPtr<rgssx::Color> obj = nullptr;
  EXC_BEGIN {
    if (argc == 0) {
      // Color.new
      obj = rgssx::MakeRefCounted<rgssx::Color>();
    } else if (argc == 3) {
      // Color.new(red, green, blue)  (alpha = 255)
      obj = rgssx::MakeRefCounted<rgssx::Color>(
          static_cast<float>(mrb_as_float(mrb, args[0])),
          static_cast<float>(mrb_as_float(mrb, args[1])),
          static_cast<float>(mrb_as_float(mrb, args[2])));
    } else if (argc == 4) {
      // Color.new(red, green, blue, alpha)
      obj = rgssx::MakeRefCounted<rgssx::Color>(
          static_cast<float>(mrb_as_float(mrb, args[0])),
          static_cast<float>(mrb_as_float(mrb, args[1])),
          static_cast<float>(mrb_as_float(mrb, args[2])),
          static_cast<float>(mrb_as_float(mrb, args[3])));
    } else {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
    }
  } EXC_END(mrb);

  SetupSelfData(self, obj.get(), kColorDataType);
  return self;
}

MRB_FUNC(Color_Set) {
  auto* self_obj = GetSelfData<rgssx::Color>(self);
  const mrb_value* args;
  mrb_int argc;
  mrb_get_args(mrb, "*", &args, &argc);

  EXC_BEGIN {
    if (argc == 1) {
      // set(color)
      self_obj->Set(GetObject<rgssx::Color>(mrb, args[0], kColorDataType));
    } else if (argc == 3) {
      // set(red, green, blue)  (alpha = 255)
      self_obj->Set(static_cast<float>(mrb_as_float(mrb, args[0])),
                    static_cast<float>(mrb_as_float(mrb, args[1])),
                    static_cast<float>(mrb_as_float(mrb, args[2])));
    } else if (argc == 4) {
      // set(red, green, blue, alpha)
      self_obj->Set(static_cast<float>(mrb_as_float(mrb, args[0])),
                    static_cast<float>(mrb_as_float(mrb, args[1])),
                    static_cast<float>(mrb_as_float(mrb, args[2])),
                    static_cast<float>(mrb_as_float(mrb, args[3])));
    } else {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
    }
  } EXC_END(mrb);
  return mrb_nil_value();
}

#define COLOR_PROP_FLOAT(cap)                                            \
  MRB_FUNC(Color_##cap) {                                                \
    auto* self_obj = GetSelfData<rgssx::Color>(self);                    \
    return mrb_float_value(mrb, static_cast<mrb_float>(self_obj->cap));  \
  }                                                                      \
  MRB_FUNC(Color_##cap##Equal) {                                         \
    auto* self_obj = GetSelfData<rgssx::Color>(self);                    \
    mrb_float value;                                                     \
    mrb_get_args(mrb, "f", &value);                                      \
    self_obj->cap = static_cast<float>(value);                           \
    return mrb_nil_value();                                              \
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
  auto* self_obj = GetSelfData<rgssx::Color>(self);
  mrb_int limit;
  mrb_get_args(mrb, "i", &limit);

  EXC_BEGIN {
    auto result =
        rgssx::Color::MarshalDump(rgssx::RefPtr<rgssx::Color>(self_obj));
    return mrb_str_new(mrb, result.data(),
                       static_cast<mrb_int>(result.size()));
  } EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Color__load) {
  mrb_value data;
  mrb_get_args(mrb, "o", &data);

  rgssx::RefPtr<rgssx::Color> obj = nullptr;
  EXC_BEGIN {
    obj = rgssx::Color::MarshalLoad(MRBStringValue(data));
  } EXC_END(mrb);

  return WrapObject(mrb, obj.get(), kColorDataType);
}

void InitColorBinding(mrb_state* mrb) {
  auto klass = DefineClass(mrb, "Color");

  mrb_define_method(mrb, klass, "initialize", Color_initialize, MRB_ARGS_ANY());
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
