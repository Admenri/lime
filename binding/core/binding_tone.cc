#include "binding_tone.h"

#include "src/utility.h"

namespace binding {

// Define mrb data type
MRB_DATATYPE_DEFINE(Tone);

MRB_FUNC(Tone_initialize) {
  mrb_int argc = mrb_get_argc(mrb);

  lime::RefPtr<lime::Tone> obj = nullptr;
  EXC_BEGIN {
    if (argc == 0) {
      // Tone.new
      obj = lime::MakeRefCounted<lime::Tone>();
    } else if (argc == 3) {
      // Tone.new(red, green, blue)  (gray = 0)
      mrb_float r, g, b;
      mrb_get_args(mrb, "fff", &r, &g, &b);
      obj = lime::MakeRefCounted<lime::Tone>(r, g, b);
    } else if (argc == 4) {
      // Tone.new(red, green, blue, gray)
      mrb_float r, g, b, a;
      mrb_get_args(mrb, "ffff", &r, &g, &b, &a);
      obj = lime::MakeRefCounted<lime::Tone>(r, g, b, a);
    } else {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
    }
  }
  EXC_END(mrb);

  return SetupSelfData(self, obj.get(), kToneDataType);
}

MRB_FUNC(Tone_initialize_copy) {
  mrb_value other;
  mrb_get_args(mrb, "o", &other);

  lime::RefPtr<lime::Tone> obj = nullptr;
  EXC_BEGIN {
    auto other_obj = GetObject<lime::Tone>(mrb, other, kToneDataType);
    obj = lime::MakeRefCounted<lime::Tone>(other_obj);
  }
  EXC_END(mrb);

  return SetupSelfData(self, obj.get(), kToneDataType);
}

MRB_FUNC(Tone_Set) {
  auto* self_obj = GetSelfData<lime::Tone>(self);

  mrb_int argc = mrb_get_argc(mrb);

  EXC_BEGIN {
    if (argc == 1) {
      // set(tone)
      mrb_value tone_val;
      mrb_get_args(mrb, "o", &tone_val);
      self_obj->Set(GetObject<lime::Tone>(mrb, tone_val, kToneDataType));
    } else if (argc == 3) {
      // set(red, green, blue)  (gray = 0)
      mrb_float r, g, b;
      mrb_get_args(mrb, "fff", &r, &g, &b);
      self_obj->Set(static_cast<float>(r), static_cast<float>(g),
                    static_cast<float>(b));
    } else if (argc == 4) {
      // set(red, green, blue, gray)
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

#define TONE_PROP_FLOAT(cap)                                            \
  MRB_FUNC(Tone_##cap) {                                                \
    auto* self_obj = GetSelfData<lime::Tone>(self);                     \
    return mrb_float_value(mrb, static_cast<mrb_float>(self_obj->cap)); \
  }                                                                     \
  MRB_FUNC(Tone_##cap##Equal) {                                         \
    auto* self_obj = GetSelfData<lime::Tone>(self);                     \
    mrb_float value;                                                    \
    mrb_get_args(mrb, "f", &value);                                     \
    self_obj->cap = static_cast<float>(value);                          \
    return mrb_nil_value();                                             \
  }

TONE_PROP_FLOAT(red);
TONE_PROP_FLOAT(green);
TONE_PROP_FLOAT(blue);
TONE_PROP_FLOAT(gray);

#undef TONE_PROP_FLOAT

// Marshal serialization (instance method _dump) / deserialization (class
// method _load). Per bindgen.md: classes with MARSHAL_DUMP/MARSHAL_LOAD get
// _dump (method) and _load (class method).
MRB_FUNC(Tone__dump) {
  auto* self_obj = GetSelfData<lime::Tone>(self);
  mrb_int limit;
  mrb_get_args(mrb, "i", &limit);

  EXC_BEGIN {
    auto result = lime::Tone::MarshalDump(lime::RefPtr<lime::Tone>(self_obj));
    return mrb_str_new(mrb, result.data(), static_cast<mrb_int>(result.size()));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Tone__load) {
  mrb_value data;
  mrb_get_args(mrb, "o", &data);

  lime::RefPtr<lime::Tone> obj = nullptr;
  EXC_BEGIN {
    obj = lime::Tone::MarshalLoad(MRBStringValue(data));
  }
  EXC_END(mrb);

  return WrapObject(mrb, obj.get(), kToneDataType);
}

void InitToneBinding(mrb_state* mrb) {
  auto klass = DefineClass(mrb, "Tone");

  mrb_define_method(mrb, klass, "initialize", Tone_initialize, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "initialize_copy", Tone_initialize_copy,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "set", Tone_Set, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "red", Tone_red, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "red=", Tone_redEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "green", Tone_green, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "green=", Tone_greenEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "blue", Tone_blue, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "blue=", Tone_blueEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "gray", Tone_gray, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "gray=", Tone_grayEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "_dump", Tone__dump, MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, klass, "_load", Tone__load, MRB_ARGS_REQ(1));
}

}  // namespace binding
