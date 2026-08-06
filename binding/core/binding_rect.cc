#include "binding_rect.h"

#include "src/utility.h"

namespace binding {

// Define mrb data type
MRB_DATATYPE_DEFINE(Rect);

MRB_FUNC(Rect_initialize) {
  const mrb_value* args;
  mrb_int argc;
  mrb_get_args(mrb, "*", &args, &argc);

  rgssx::RefPtr<rgssx::Rect> obj = nullptr;
  EXC_BEGIN {
    if (argc == 4) {
      // Rect.new(x, y, width, height)
      obj = rgssx::MakeRefCounted<rgssx::Rect>(
          mrb_integer(args[0]), mrb_integer(args[1]), mrb_integer(args[2]),
          mrb_integer(args[3]));
    } else if (argc == 0) {
      // Rect.new
      obj = rgssx::MakeRefCounted<rgssx::Rect>();
    } else {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
    }
  }
  EXC_END(mrb);

  SetupSelfData(self, obj.get(), kRectDataType);
  return self;
}

MRB_FUNC(Rect_initialize_copy) {
  mrb_value other;
  mrb_get_args(mrb, "o", &other);

  rgssx::RefPtr<rgssx::Rect> obj = nullptr;
  EXC_BEGIN {
    auto other_obj = GetObject<rgssx::Rect>(mrb, other, kRectDataType);
    obj = rgssx::MakeRefCounted<rgssx::Rect>(other_obj);
  }
  EXC_END(mrb);

  SetupSelfData(self, obj.get(), kRectDataType);
  return self;
}

MRB_FUNC(Rect_Set) {
  auto* self_obj = GetSelfData<rgssx::Rect>(self);
  const mrb_value* args;
  mrb_int argc;
  mrb_get_args(mrb, "*", &args, &argc);

  EXC_BEGIN {
    if (argc == 4) {
      // set(x, y, width, height)
      self_obj->Set(mrb_integer(args[0]), mrb_integer(args[1]),
                    mrb_integer(args[2]), mrb_integer(args[3]));
    } else if (argc == 1) {
      // set(rect)
      self_obj->Set(GetObject<rgssx::Rect>(mrb, args[0], kRectDataType));
    } else {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
    }
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Rect_Empty) {
  auto* self_obj = GetSelfData<rgssx::Rect>(self);
  EXC_BEGIN {
    self_obj->Empty();
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

#define RECT_PROP_INT(cap)                           \
  MRB_FUNC(Rect_##cap) {                             \
    auto* self_obj = GetSelfData<rgssx::Rect>(self); \
    return mrb_fixnum_value(self_obj->cap);          \
  }                                                  \
  MRB_FUNC(Rect_##cap##Equal) {                      \
    auto* self_obj = GetSelfData<rgssx::Rect>(self); \
    mrb_int value;                                   \
    mrb_get_args(mrb, "i", &value);                  \
    self_obj->cap = static_cast<int>(value);         \
    return mrb_nil_value();                          \
  }

RECT_PROP_INT(x);
RECT_PROP_INT(y);
RECT_PROP_INT(width);
RECT_PROP_INT(height);

#undef RECT_PROP_INT

// Marshal serialization (instance method _dump) / deserialization (class
// method _load). Per bindgen.md: classes with MARSHAL_DUMP/MARSHAL_LOAD get
// _dump (method) and _load (class method).
MRB_FUNC(Rect__dump) {
  auto* self_obj = GetSelfData<rgssx::Rect>(self);
  mrb_int limit;
  mrb_get_args(mrb, "i", &limit);

  EXC_BEGIN {
    auto result =
        rgssx::Rect::MarshalDump(rgssx::RefPtr<rgssx::Rect>(self_obj));
    return mrb_str_new(mrb, result.data(), static_cast<mrb_int>(result.size()));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Rect__load) {
  mrb_value data;
  mrb_get_args(mrb, "o", &data);

  rgssx::RefPtr<rgssx::Rect> obj = nullptr;
  EXC_BEGIN {
    obj = rgssx::Rect::MarshalLoad(MRBStringValue(data));
  }
  EXC_END(mrb);

  return WrapObject(mrb, obj.get(), kRectDataType);
}

void InitRectBinding(mrb_state* mrb) {
  auto klass = DefineClass(mrb, "Rect");

  mrb_define_method(mrb, klass, "initialize", Rect_initialize, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "initialize_copy", Rect_initialize_copy,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "set", Rect_Set, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "empty", Rect_Empty, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "x", Rect_x, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "x=", Rect_xEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "y", Rect_y, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "y=", Rect_yEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "width", Rect_width, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "width=", Rect_widthEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "height", Rect_height, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "height=", Rect_heightEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "_dump", Rect__dump, MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, klass, "_load", Rect__load, MRB_ARGS_REQ(1));
}

}  // namespace binding
