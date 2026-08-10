#include "binding_shader.h"

#include "binding_bitmap.h"

#include "src/bitmap.h"
#include "src/shader.h"

namespace binding {

// Define mrb data type
MRB_DATATYPE_DEFINE(Shader);

// Helper: convert a Ruby Array of numbers to std::vector<float>.
static std::vector<float> GetFloatArray(mrb_state* mrb, mrb_value val) {
  std::vector<float> result;
  if (mrb_nil_p(val))
    return result;
  if (!mrb_array_p(val))
    mrb_raise(mrb, E_TYPE_ERROR, "expected Array");
  mrb_int len = RARRAY_LEN(val);
  mrb_value* ptr = RARRAY_PTR(val);
  result.reserve(static_cast<size_t>(len));
  for (mrb_int i = 0; i < len; ++i)
    result.push_back(static_cast<float>(mrb_as_float(mrb, ptr[i])));
  return result;
}

// Helper: convert a Ruby Array of numbers to std::vector<int32_t>.
static std::vector<int32_t> GetIntArray(mrb_state* mrb, mrb_value val) {
  std::vector<int32_t> result;
  if (mrb_nil_p(val))
    return result;
  if (!mrb_array_p(val))
    mrb_raise(mrb, E_TYPE_ERROR, "expected Array");
  mrb_int len = RARRAY_LEN(val);
  mrb_value* ptr = RARRAY_PTR(val);
  result.reserve(static_cast<size_t>(len));
  for (mrb_int i = 0; i < len; ++i)
    result.push_back(static_cast<int32_t>(mrb_as_int(mrb, ptr[i])));
  return result;
}

// Helper: convert a Ruby Array of numbers to std::vector<uint32_t>.
static std::vector<uint32_t> GetUIntArray(mrb_state* mrb, mrb_value val) {
  std::vector<uint32_t> result;
  if (mrb_nil_p(val))
    return result;
  if (!mrb_array_p(val))
    mrb_raise(mrb, E_TYPE_ERROR, "expected Array");
  mrb_int len = RARRAY_LEN(val);
  mrb_value* ptr = RARRAY_PTR(val);
  result.reserve(static_cast<size_t>(len));
  for (mrb_int i = 0; i < len; ++i)
    result.push_back(static_cast<uint32_t>(mrb_as_int(mrb, ptr[i])));
  return result;
}

// Helper: convert a Ruby Array of numbers into a 4x4 matrix (16 floats).
// Missing entries are left as zero.
static void GetMatrixArray(mrb_state* mrb, mrb_value val, float out[16]) {
  if (mrb_nil_p(val))
    return;
  if (!mrb_array_p(val))
    mrb_raise(mrb, E_TYPE_ERROR, "expected Array");
  mrb_int len = RARRAY_LEN(val);
  mrb_value* ptr = RARRAY_PTR(val);
  mrb_int count = len < 16 ? len : 16;
  for (mrb_int i = 0; i < count; ++i)
    out[i] = static_cast<float>(mrb_as_float(mrb, ptr[i]));
}

MRB_FUNC(Shader_initialize) {
  const char* vs_code;
  const char* fs_code;
  mrb_get_args(mrb, "zz", &vs_code, &fs_code);

  lime::RefPtr<lime::Shader> obj = nullptr;
  EXC_BEGIN {
    obj = lime::MakeRefCounted<lime::Shader>(vs_code, fs_code);
  }
  EXC_END(mrb);

  return SetupSelfData(self, obj.get(), kShaderDataType);
}

MRB_FUNC(Shader_SetValueF) {
  auto* self_obj = GetSelfData<lime::Shader>(self);

  mrb_int argc = mrb_get_argc(mrb);
  EXC_BEGIN {
    if (argc == 2) {
      // set_value_f(uniform, value)  (item_count = 1)
      const char* uniform;
      mrb_value value_val;
      mrb_get_args(mrb, "zo", &uniform, &value_val);
      auto value = GetFloatArray(mrb, value_val);
      self_obj->SetValueF(uniform, value, 1);
    } else if (argc == 3) {
      // set_value_f(uniform, value, item_count)
      const char* uniform;
      mrb_value value_val;
      mrb_int item_count;
      mrb_get_args(mrb, "zoi", &uniform, &value_val, &item_count);
      auto value = GetFloatArray(mrb, value_val);
      self_obj->SetValueF(uniform, value, item_count);
    } else {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
    }
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Shader_SetValueI) {
  auto* self_obj = GetSelfData<lime::Shader>(self);

  mrb_int argc = mrb_get_argc(mrb);
  EXC_BEGIN {
    if (argc == 2) {
      // set_value_i(uniform, value)  (item_count = 1)
      const char* uniform;
      mrb_value value_val;
      mrb_get_args(mrb, "zo", &uniform, &value_val);
      auto value = GetIntArray(mrb, value_val);
      self_obj->SetValueI(uniform, value, 1);
    } else if (argc == 3) {
      // set_value_i(uniform, value, item_count)
      const char* uniform;
      mrb_value value_val;
      mrb_int item_count;
      mrb_get_args(mrb, "zoi", &uniform, &value_val, &item_count);
      auto value = GetIntArray(mrb, value_val);
      self_obj->SetValueI(uniform, value, item_count);
    } else {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
    }
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Shader_SetValueU) {
  auto* self_obj = GetSelfData<lime::Shader>(self);

  mrb_int argc = mrb_get_argc(mrb);
  EXC_BEGIN {
    if (argc == 2) {
      // set_value_u(uniform, value)  (item_count = 1)
      const char* uniform;
      mrb_value value_val;
      mrb_get_args(mrb, "zo", &uniform, &value_val);
      auto value = GetUIntArray(mrb, value_val);
      self_obj->SetValueU(uniform, value, 1);
    } else if (argc == 3) {
      // set_value_u(uniform, value, item_count)
      const char* uniform;
      mrb_value value_val;
      mrb_int item_count;
      mrb_get_args(mrb, "zoi", &uniform, &value_val, &item_count);
      auto value = GetUIntArray(mrb, value_val);
      self_obj->SetValueU(uniform, value, item_count);
    } else {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
    }
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Shader_SetValueT) {
  auto* self_obj = GetSelfData<lime::Shader>(self);
  const char* uniform;
  mrb_value texture_val;
  mrb_get_args(mrb, "zo", &uniform, &texture_val);

  auto texture = GetObject<lime::Bitmap>(mrb, texture_val, kBitmapDataType);

  EXC_BEGIN {
    self_obj->SetValueT(uniform, texture);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Shader_SetValueM) {
  auto* self_obj = GetSelfData<lime::Shader>(self);
  const char* uniform;
  mrb_value value_val;
  mrb_get_args(mrb, "zo", &uniform, &value_val);

  float value[16] = {};
  GetMatrixArray(mrb, value_val, value);

  EXC_BEGIN {
    self_obj->SetValueM(uniform, value);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

void InitShaderBinding(mrb_state* mrb) {
  auto klass = DefineClass(mrb, "Shader");

  mrb_define_method(mrb, klass, "initialize", Shader_initialize,
                    MRB_ARGS_REQ(2));
  mrb_define_method(mrb, klass, "set_value_f", Shader_SetValueF,
                    MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "set_value_i", Shader_SetValueI,
                    MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "set_value_u", Shader_SetValueU,
                    MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "set_value_t", Shader_SetValueT,
                    MRB_ARGS_REQ(2));
  mrb_define_method(mrb, klass, "set_value_m", Shader_SetValueM,
                    MRB_ARGS_REQ(2));
}

}  // namespace binding
