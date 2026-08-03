#include "binding_tone.h"

#include "src/utility.h"

namespace binding {

// Define mrb data type
MRB_DATATYPE_DEFINE(Tone);

MRB_FUNC(Tone_initialize) {
  const mrb_value* args;
  mrb_int argc;
  mrb_get_args(mrb, "*", &args, &argc);

  rgssx::RefPtr<rgssx::Tone> obj = nullptr;
  EXC_BEGIN {
    if (argc == 0) {
      // Tone.new
      obj = rgssx::MakeRefCounted<rgssx::Tone>();
    } else if (argc == 3) {
      // Tone.new(red, green, blue)  (gray = 0)
      obj = rgssx::MakeRefCounted<rgssx::Tone>(
          static_cast<float>(mrb_as_float(mrb, args[0])),
          static_cast<float>(mrb_as_float(mrb, args[1])),
          static_cast<float>(mrb_as_float(mrb, args[2])));
    } else if (argc == 4) {
      // Tone.new(red, green, blue, gray)
      obj = rgssx::MakeRefCounted<rgssx::Tone>(
          static_cast<float>(mrb_as_float(mrb, args[0])),
          static_cast<float>(mrb_as_float(mrb, args[1])),
          static_cast<float>(mrb_as_float(mrb, args[2])),
          static_cast<float>(mrb_as_float(mrb, args[3])));
    } else {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
    }
  } EXC_END(mrb);

  SetupSelfData(self, obj.get(), kToneDataType);
  return self;
}

MRB_FUNC(Tone_Set) {
  auto* self_obj = GetSelfData<rgssx::Tone>(self);
  const mrb_value* args;
  mrb_int argc;
  mrb_get_args(mrb, "*", &args, &argc);

  EXC_BEGIN {
    if (argc == 1) {
      // set(tone)
      self_obj->Set(GetObject<rgssx::Tone>(mrb, args[0], kToneDataType));
    } else if (argc == 3) {
      // set(red, green, blue)  (gray = 0)
      self_obj->Set(static_cast<float>(mrb_as_float(mrb, args[0])),
                    static_cast<float>(mrb_as_float(mrb, args[1])),
                    static_cast<float>(mrb_as_float(mrb, args[2])));
    } else if (argc == 4) {
      // set(red, green, blue, gray)
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

#define TONE_PROP_FLOAT(cap)                                             \
  MRB_FUNC(Tone_##cap) {                                                 \
    auto* self_obj = GetSelfData<rgssx::Tone>(self);                     \
    return mrb_float_value(mrb, static_cast<mrb_float>(self_obj->cap));  \
  }                                                                      \
  MRB_FUNC(Tone_##cap##Equal) {                                          \
    auto* self_obj = GetSelfData<rgssx::Tone>(self);                     \
    mrb_float value;                                                     \
    mrb_get_args(mrb, "f", &value);                                      \
    self_obj->cap = static_cast<float>(value);                           \
    return mrb_nil_value();                                              \
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
  auto* self_obj = GetSelfData<rgssx::Tone>(self);
  mrb_int limit;
  mrb_get_args(mrb, "i", &limit);

  EXC_BEGIN {
    auto result =
        rgssx::Tone::MarshalDump(rgssx::RefPtr<rgssx::Tone>(self_obj));
    return mrb_str_new(mrb, result.data(),
                       static_cast<mrb_int>(result.size()));
  } EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Tone__load) {
  mrb_value data;
  mrb_get_args(mrb, "o", &data);

  rgssx::RefPtr<rgssx::Tone> obj = nullptr;
  EXC_BEGIN {
    obj = rgssx::Tone::MarshalLoad(MRBStringValue(data));
  } EXC_END(mrb);

  return WrapObject(mrb, obj.get(), kToneDataType);
}

void InitToneBinding(mrb_state* mrb) {
  auto klass = DefineClass(mrb, "Tone");

  mrb_define_method(mrb, klass, "initialize", Tone_initialize, MRB_ARGS_ANY());
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
