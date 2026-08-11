#include "binding_vector.h"

#include "src/utility.h"

namespace binding {

// Define mrb data types
MRB_DATATYPE_DEFINE(Vector2);
MRB_DATATYPE_DEFINE(Vector3);
MRB_DATATYPE_DEFINE(Vector4);

// ---------------------------------------------------------------------------
// Vector2
// ---------------------------------------------------------------------------

MRB_FUNC(Vector2_initialize) {
  mrb_int argc = mrb_get_argc(mrb);

  lime::RefPtr<lime::Vector2> obj = nullptr;
  EXC_BEGIN {
    if (argc == 0) {
      // Vector2.new
      obj = lime::MakeRefCounted<lime::Vector2>();
    } else if (argc == 2) {
      // Vector2.new(x, y)
      mrb_float x, y;
      mrb_get_args(mrb, "ff", &x, &y);
      obj = lime::MakeRefCounted<lime::Vector2>(x, y);
    } else {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
    }
  }
  EXC_END(mrb);

  return SetupSelfData(self, obj.get(), kVector2DataType);
}

MRB_FUNC(Vector2_initialize_copy) {
  mrb_value other;
  mrb_get_args(mrb, "o", &other);

  lime::RefPtr<lime::Vector2> obj = nullptr;
  EXC_BEGIN {
    auto other_obj = GetObject<lime::Vector2>(mrb, other, kVector2DataType);
    obj = lime::MakeRefCounted<lime::Vector2>(other_obj);
  }
  EXC_END(mrb);

  return SetupSelfData(self, obj.get(), kVector2DataType);
}

MRB_FUNC(Vector2_Set) {
  auto* self_obj = GetSelfData<lime::Vector2>(self);

  mrb_int argc = mrb_get_argc(mrb);

  EXC_BEGIN {
    if (argc == 1) {
      // set(vector2)
      mrb_value vec_val;
      mrb_get_args(mrb, "o", &vec_val);
      self_obj->Set(GetObject<lime::Vector2>(mrb, vec_val, kVector2DataType));
    } else if (argc == 2) {
      // set(x, y)
      mrb_float x, y;
      mrb_get_args(mrb, "ff", &x, &y);
      self_obj->Set(static_cast<float>(x), static_cast<float>(y));
    } else {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
    }
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

#define VECTOR2_PROP_FLOAT(cap)                                         \
  MRB_FUNC(Vector2_##cap) {                                             \
    auto* self_obj = GetSelfData<lime::Vector2>(self);                  \
    return mrb_float_value(mrb, static_cast<mrb_float>(self_obj->cap)); \
  }                                                                     \
  MRB_FUNC(Vector2_##cap##Equal) {                                      \
    auto* self_obj = GetSelfData<lime::Vector2>(self);                  \
    mrb_float value;                                                    \
    mrb_get_args(mrb, "f", &value);                                     \
    self_obj->cap = static_cast<float>(value);                          \
    return mrb_nil_value();                                             \
  }

VECTOR2_PROP_FLOAT(x);
VECTOR2_PROP_FLOAT(y);

#undef VECTOR2_PROP_FLOAT

// ---------------------------------------------------------------------------
// Vector3
// ---------------------------------------------------------------------------

MRB_FUNC(Vector3_initialize) {
  mrb_int argc = mrb_get_argc(mrb);

  lime::RefPtr<lime::Vector3> obj = nullptr;
  EXC_BEGIN {
    if (argc == 0) {
      // Vector3.new
      obj = lime::MakeRefCounted<lime::Vector3>();
    } else if (argc == 3) {
      // Vector3.new(x, y, z)
      mrb_float x, y, z;
      mrb_get_args(mrb, "fff", &x, &y, &z);
      obj = lime::MakeRefCounted<lime::Vector3>(x, y, z);
    } else {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
    }
  }
  EXC_END(mrb);

  return SetupSelfData(self, obj.get(), kVector3DataType);
}

MRB_FUNC(Vector3_initialize_copy) {
  mrb_value other;
  mrb_get_args(mrb, "o", &other);

  lime::RefPtr<lime::Vector3> obj = nullptr;
  EXC_BEGIN {
    auto other_obj = GetObject<lime::Vector3>(mrb, other, kVector3DataType);
    obj = lime::MakeRefCounted<lime::Vector3>(other_obj);
  }
  EXC_END(mrb);

  return SetupSelfData(self, obj.get(), kVector3DataType);
}

MRB_FUNC(Vector3_Set) {
  auto* self_obj = GetSelfData<lime::Vector3>(self);

  mrb_int argc = mrb_get_argc(mrb);

  EXC_BEGIN {
    if (argc == 1) {
      // set(vector3)
      mrb_value vec_val;
      mrb_get_args(mrb, "o", &vec_val);
      self_obj->Set(GetObject<lime::Vector3>(mrb, vec_val, kVector3DataType));
    } else if (argc == 3) {
      // set(x, y, z)
      mrb_float x, y, z;
      mrb_get_args(mrb, "fff", &x, &y, &z);
      self_obj->Set(static_cast<float>(x), static_cast<float>(y),
                    static_cast<float>(z));
    } else {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
    }
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

#define VECTOR3_PROP_FLOAT(cap)                                         \
  MRB_FUNC(Vector3_##cap) {                                             \
    auto* self_obj = GetSelfData<lime::Vector3>(self);                  \
    return mrb_float_value(mrb, static_cast<mrb_float>(self_obj->cap)); \
  }                                                                     \
  MRB_FUNC(Vector3_##cap##Equal) {                                      \
    auto* self_obj = GetSelfData<lime::Vector3>(self);                  \
    mrb_float value;                                                    \
    mrb_get_args(mrb, "f", &value);                                     \
    self_obj->cap = static_cast<float>(value);                          \
    return mrb_nil_value();                                             \
  }

VECTOR3_PROP_FLOAT(x);
VECTOR3_PROP_FLOAT(y);
VECTOR3_PROP_FLOAT(z);

#undef VECTOR3_PROP_FLOAT

// ---------------------------------------------------------------------------
// Vector4
// ---------------------------------------------------------------------------

MRB_FUNC(Vector4_initialize) {
  mrb_int argc = mrb_get_argc(mrb);

  lime::RefPtr<lime::Vector4> obj = nullptr;
  EXC_BEGIN {
    if (argc == 0) {
      // Vector4.new
      obj = lime::MakeRefCounted<lime::Vector4>();
    } else if (argc == 4) {
      // Vector4.new(x, y, z, w)
      mrb_float x, y, z, w;
      mrb_get_args(mrb, "ffff", &x, &y, &z, &w);
      obj = lime::MakeRefCounted<lime::Vector4>(x, y, z, w);
    } else {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
    }
  }
  EXC_END(mrb);

  return SetupSelfData(self, obj.get(), kVector4DataType);
}

MRB_FUNC(Vector4_initialize_copy) {
  mrb_value other;
  mrb_get_args(mrb, "o", &other);

  lime::RefPtr<lime::Vector4> obj = nullptr;
  EXC_BEGIN {
    auto other_obj = GetObject<lime::Vector4>(mrb, other, kVector4DataType);
    obj = lime::MakeRefCounted<lime::Vector4>(other_obj);
  }
  EXC_END(mrb);

  return SetupSelfData(self, obj.get(), kVector4DataType);
}

MRB_FUNC(Vector4_Set) {
  auto* self_obj = GetSelfData<lime::Vector4>(self);

  mrb_int argc = mrb_get_argc(mrb);

  EXC_BEGIN {
    if (argc == 1) {
      // set(vector4)
      mrb_value vec_val;
      mrb_get_args(mrb, "o", &vec_val);
      self_obj->Set(GetObject<lime::Vector4>(mrb, vec_val, kVector4DataType));
    } else if (argc == 4) {
      // set(x, y, z, w)
      mrb_float x, y, z, w;
      mrb_get_args(mrb, "ffff", &x, &y, &z, &w);
      self_obj->Set(static_cast<float>(x), static_cast<float>(y),
                    static_cast<float>(z), static_cast<float>(w));
    } else {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
    }
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

#define VECTOR4_PROP_FLOAT(cap)                                         \
  MRB_FUNC(Vector4_##cap) {                                             \
    auto* self_obj = GetSelfData<lime::Vector4>(self);                  \
    return mrb_float_value(mrb, static_cast<mrb_float>(self_obj->cap)); \
  }                                                                     \
  MRB_FUNC(Vector4_##cap##Equal) {                                      \
    auto* self_obj = GetSelfData<lime::Vector4>(self);                  \
    mrb_float value;                                                    \
    mrb_get_args(mrb, "f", &value);                                     \
    self_obj->cap = static_cast<float>(value);                          \
    return mrb_nil_value();                                             \
  }

VECTOR4_PROP_FLOAT(x);
VECTOR4_PROP_FLOAT(y);
VECTOR4_PROP_FLOAT(z);
VECTOR4_PROP_FLOAT(w);

#undef VECTOR4_PROP_FLOAT

void InitVectorBinding(mrb_state* mrb) {
  // Vector2
  auto klass2 = DefineClass(mrb, "Vector2");
  mrb_define_method(mrb, klass2, "initialize", Vector2_initialize,
                    MRB_ARGS_ANY());
  mrb_define_method(mrb, klass2, "initialize_copy", Vector2_initialize_copy,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass2, "set", Vector2_Set, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass2, "x", Vector2_x, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass2, "x=", Vector2_xEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass2, "y", Vector2_y, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass2, "y=", Vector2_yEqual, MRB_ARGS_REQ(1));

  // Vector3
  auto klass3 = DefineClass(mrb, "Vector3");
  mrb_define_method(mrb, klass3, "initialize", Vector3_initialize,
                    MRB_ARGS_ANY());
  mrb_define_method(mrb, klass3, "initialize_copy", Vector3_initialize_copy,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass3, "set", Vector3_Set, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass3, "x", Vector3_x, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass3, "x=", Vector3_xEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass3, "y", Vector3_y, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass3, "y=", Vector3_yEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass3, "z", Vector3_z, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass3, "z=", Vector3_zEqual, MRB_ARGS_REQ(1));

  // Vector4
  auto klass4 = DefineClass(mrb, "Vector4");
  mrb_define_method(mrb, klass4, "initialize", Vector4_initialize,
                    MRB_ARGS_ANY());
  mrb_define_method(mrb, klass4, "initialize_copy", Vector4_initialize_copy,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass4, "set", Vector4_Set, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass4, "x", Vector4_x, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass4, "x=", Vector4_xEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass4, "y", Vector4_y, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass4, "y=", Vector4_yEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass4, "z", Vector4_z, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass4, "z=", Vector4_zEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass4, "w", Vector4_w, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass4, "w=", Vector4_wEqual, MRB_ARGS_REQ(1));
}

}  // namespace binding
