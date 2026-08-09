#pragma once

#include <cctype>
#include <string>
#include <vector>

#include "mruby.h"
#include "mruby/array.h"
#include "mruby/class.h"
#include "mruby/compile.h"
#include "mruby/data.h"
#include "mruby/error.h"
#include "mruby/string.h"
#include "mruby/variable.h"
#include "mruby/version.h"

#include "src/common.h"
#include "src/refptr.h"

namespace binding {

extern RClass* g_reset_exception;
extern RClass* g_rgss_exception;
extern RClass* g_exit_exception;

// Klass define helper function.
inline RClass* DefineClass(mrb_state* mrb, const char* name) {
  auto klass = mrb_define_class(mrb, name, mrb->object_class);
  MRB_SET_INSTANCE_TT(klass, MRB_TT_CDATA);
  return klass;
}

#define MRB_FUNC(name) static mrb_value name(mrb_state* mrb, mrb_value self)

// Data helper functions.
template <typename Ty>
inline mrb_value SetupSelfData(mrb_value self,
                               Ty* data,
                               const mrb_data_type& type) {
  DATA_PTR(self) = data;
  DATA_TYPE(self) = &type;

  data->AddRef();

  return self;
}

template <typename Ty>
inline Ty* GetSelfData(mrb_value self) {
  return static_cast<Ty*>(DATA_PTR(self));
}

template <typename Ty>
inline Ty* GetSelfDataCheck(mrb_state* mrb,
                            mrb_value obj,
                            const mrb_data_type& type) {
  return static_cast<Ty*>(mrb_check_datatype(mrb, obj, &type));
}

template <typename Ty>
inline mrb_value WrapObject(mrb_state* mrb,
                            Ty* ptr,
                            const mrb_data_type& type) {
  if (!ptr)
    return mrb_nil_value();

  RClass* klass = mrb_class_get(mrb, type.struct_name);
  RData* data = mrb_data_object_alloc(mrb, klass, ptr, &type);
  mrb_value obj = mrb_obj_value(data);

  SetupSelfData(obj, ptr, type);

  return obj;
}

// Data type helper macro.
#define MRB_DATATYPE_DECLARE(type) extern const mrb_data_type k##type##DataType;
#define MRB_DATATYPE_DEFINE(type)           \
  const mrb_data_type k##type##DataType = { \
      #type,                                \
      ReleaseDataType<lime::type>,          \
  };

template <typename Ty>
inline void ReleaseDataType(mrb_state* mrb, void* ptr) {
  static_cast<Ty*>(ptr)->Release();
}

inline void ProcessException(mrb_state* mrb, const lime::Exception& e) {
  RClass* exc = mrb->eStandardError_class;
  if (e.type() == lime::Exception::ExitError)
    exc = g_exit_exception;
  if (e.type() == lime::Exception::ResetError)
    exc = g_reset_exception;
  if (e.type() == lime::Exception::RGSSError)
    exc = g_rgss_exception;
  if (e.type() == lime::Exception::IOError)
    exc = g_rgss_exception;

  mrb_raise(mrb, exc, e.message().c_str());
}

// Exception helper function.
#define EXC_BEGIN try
#define EXC_END(mrb)                 \
  catch (const lime::Exception& e) { \
    ProcessException(mrb, e);        \
  }

// Converts a CamelCase identifier to snake_case (e.g. "GetRect" -> "get_rect",
// "XSize" -> "x_size"). Used to derive Ruby method names from C++ names.
inline std::string ToSnake(const std::string& name) {
  std::string result;
  result.reserve(name.size() + 4);
  for (size_t i = 0; i < name.size(); ++i) {
    char c = name[i];
    if (std::isupper(static_cast<unsigned char>(c))) {
      if (i > 0 && (std::islower(static_cast<unsigned char>(name[i - 1])) ||
                    (i + 1 < name.size() &&
                     std::islower(static_cast<unsigned char>(name[i + 1])) &&
                     std::isupper(static_cast<unsigned char>(name[i - 1])))))
        result += '_';
      result += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    } else {
      result += c;
    }
  }
  return result;
}

// Unwraps a wrapped ref-counted object from an mrb_value. Passing nil yields a
// null RefPtr; passing an object of the wrong type raises TypeError.
template <typename Ty>
inline lime::RefPtr<Ty> GetObject(mrb_state* mrb,
                                  mrb_value val,
                                  const mrb_data_type& type) {
  if (mrb_nil_p(val))
    return nullptr;
  auto* ptr = static_cast<Ty*>(mrb_data_get_ptr(mrb, val, &type));
  return lime::RefPtr<Ty>(ptr);
}

// Ruby Array of Strings (or a single String, or nil) <->
// std::vector<std::string>. A bare String is treated as a one-element array
// (RGSS allows both forms, e.g. Font.default_name = "SimHei" or
// Font.default_name = ["Arial", "SimHei"]). Any other type raises a TypeError
// instead of reading garbage lengths.
inline std::vector<std::string> GetStringVector(mrb_state* mrb, mrb_value val) {
  std::vector<std::string> result;
  if (mrb_nil_p(val))
    return result;

  if (mrb_string_p(val)) {
    result.emplace_back(mrb_str_to_cstr(mrb, val));
    return result;
  }

  if (!mrb_array_p(val))
    mrb_raise(mrb, E_TYPE_ERROR, "expected Array or String");

  mrb_int len = RARRAY_LEN(val);
  mrb_value* ptr = RARRAY_PTR(val);
  result.reserve(static_cast<size_t>(len));
  for (mrb_int i = 0; i < len; ++i) {
    if (!mrb_string_p(ptr[i]))
      mrb_raise(mrb, E_TYPE_ERROR, "expected String elements");
    result.emplace_back(mrb_str_to_cstr(mrb, ptr[i]));
  }
  return result;
}

inline mrb_value WrapStringVector(mrb_state* mrb,
                                  const std::vector<std::string>& vec) {
  mrb_value ary = mrb_ary_new_capa(mrb, static_cast<mrb_int>(vec.size()));
  for (const auto& s : vec)
    mrb_ary_push(mrb, ary, mrb_str_new_cstr(mrb, s.c_str()));
  return ary;
}

// Generic ATTR binding helpers: generate a getter and a setter for an
// attribute. "prefix" is the binding function prefix (e.g. Plane), "ty" the
// C++ class (e.g. lime::Plane), "cap" the attribute name (e.g. ZoomX).
#define BINDING_ATTR_INT(prefix, ty, cap)            \
  MRB_FUNC(prefix##_##cap) {                         \
    auto* self_obj = GetSelfData<ty>(self);          \
    EXC_BEGIN {                                      \
      auto result = self_obj->Attr_##cap();          \
      return mrb_fixnum_value(*result);              \
    }                                                \
    EXC_END(mrb);                                    \
    return mrb_nil_value();                          \
  }                                                  \
  MRB_FUNC(prefix##_##cap##Equal) {                  \
    auto* self_obj = GetSelfData<ty>(self);          \
    mrb_int value;                                   \
    mrb_get_args(mrb, "i", &value);                  \
    EXC_BEGIN {                                      \
      self_obj->Attr_##cap(static_cast<int>(value)); \
    }                                                \
    EXC_END(mrb);                                    \
    return mrb_nil_value();                          \
  }

#define BINDING_ATTR_FLOAT(prefix, ty, cap)                         \
  MRB_FUNC(prefix##_##cap) {                                        \
    auto* self_obj = GetSelfData<ty>(self);                         \
    EXC_BEGIN {                                                     \
      auto result = self_obj->Attr_##cap();                         \
      return mrb_float_value(mrb, static_cast<mrb_float>(*result)); \
    }                                                               \
    EXC_END(mrb);                                                   \
    return mrb_nil_value();                                         \
  }                                                                 \
  MRB_FUNC(prefix##_##cap##Equal) {                                 \
    auto* self_obj = GetSelfData<ty>(self);                         \
    mrb_float value;                                                \
    mrb_get_args(mrb, "f", &value);                                 \
    EXC_BEGIN {                                                     \
      self_obj->Attr_##cap(static_cast<float>(value));              \
    }                                                               \
    EXC_END(mrb);                                                   \
    return mrb_nil_value();                                         \
  }

#define BINDING_ATTR_BOOL(prefix, ty, cap)  \
  MRB_FUNC(prefix##_##cap) {                \
    auto* self_obj = GetSelfData<ty>(self); \
    EXC_BEGIN {                             \
      auto result = self_obj->Attr_##cap(); \
      return mrb_bool_value(*result);       \
    }                                       \
    EXC_END(mrb);                           \
    return mrb_nil_value();                 \
  }                                         \
  MRB_FUNC(prefix##_##cap##Equal) {         \
    auto* self_obj = GetSelfData<ty>(self); \
    mrb_bool value;                         \
    mrb_get_args(mrb, "b", &value);         \
    EXC_BEGIN {                             \
      self_obj->Attr_##cap(value);          \
    }                                       \
    EXC_END(mrb);                           \
    return mrb_nil_value();                 \
  }

// Object attribute. objty is the object type (e.g. lime::Bitmap), dataty is
// the mrb data type constant (e.g. kBitmapDataType).
#define BINDING_ATTR_OBJECT(prefix, ty, cap, objty, dataty) \
  MRB_FUNC(prefix##_##cap) {                                \
    auto* self_obj = GetSelfData<ty>(self);                 \
    EXC_BEGIN {                                             \
      auto result = self_obj->Attr_##cap();                 \
      return WrapObject(mrb, result->get(), dataty);        \
    }                                                       \
    EXC_END(mrb);                                           \
    return mrb_nil_value();                                 \
  }                                                         \
  MRB_FUNC(prefix##_##cap##Equal) {                         \
    auto* self_obj = GetSelfData<ty>(self);                 \
    mrb_value val;                                          \
    mrb_get_args(mrb, "o", &val);                           \
    auto obj = GetObject<objty>(mrb, val, dataty);          \
    EXC_BEGIN {                                             \
      self_obj->Attr_##cap(obj);                            \
    }                                                       \
    EXC_END(mrb);                                           \
    return mrb_nil_value();                                 \
  }

// Reference object attribute.
#define BINDING_ATTR_OBJECT_REF(prefix, ty, cap, objty, dataty)               \
  MRB_FUNC(prefix##_##cap) {                                                  \
    auto* self_obj = GetSelfData<ty>(self);                                   \
    EXC_BEGIN {                                                               \
      auto result = self_obj->Attr_##cap();                                   \
      auto result_iv = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@_" #cap)); \
      if (!mrb_nil_p(result_iv))                                              \
        return result_iv;                                                     \
      auto result_obj = WrapObject(mrb, result->get(), dataty);               \
      mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "@_" #cap), result_obj);      \
      return result_obj;                                                      \
    }                                                                         \
    EXC_END(mrb);                                                             \
    return mrb_nil_value();                                                   \
  }                                                                           \
  MRB_FUNC(prefix##_##cap##Equal) {                                           \
    auto* self_obj = GetSelfData<ty>(self);                                   \
    mrb_value val;                                                            \
    mrb_get_args(mrb, "o", &val);                                             \
    auto obj = GetObject<objty>(mrb, val, dataty);                            \
    EXC_BEGIN {                                                               \
      self_obj->Attr_##cap(obj);                                              \
      mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "@_" #cap), val);             \
    }                                                                         \
    EXC_END(mrb);                                                             \
    return mrb_nil_value();                                                   \
  }

// Inherited Dispoable methods (IsDisposed / Dispose). Per the bindgen rules,
// the parent class's exported section is merged into the derived class.
#define BINDING_INHERITED_DISPOABLE(prefix, ty)      \
  MRB_FUNC(prefix##_IsDisposed) {                    \
    auto* self_obj = GetSelfData<ty>(self);          \
    EXC_BEGIN {                                      \
      return mrb_bool_value(self_obj->IsDisposed()); \
    }                                                \
    EXC_END(mrb);                                    \
    return mrb_nil_value();                          \
  }                                                  \
  MRB_FUNC(prefix##_Dispose) {                       \
    auto* self_obj = GetSelfData<ty>(self);          \
    EXC_BEGIN {                                      \
      self_obj->Dispose();                           \
    }                                                \
    EXC_END(mrb);                                    \
    return mrb_nil_value();                          \
  }

inline std::string MRBStringValue(mrb_value str) {
  if (mrb_string_p(str))
    return std::string(RSTRING_PTR(str), RSTRING_LEN(str));
  return {};
}

}  // namespace binding
