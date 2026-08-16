#include "binding_effect.h"

#include "binding_bitmap.h"

#include "mruby/hash.h"

#include "src/bitmap.h"
#include "src/effect.h"

namespace binding {

// Define mrb data type
MRB_DATATYPE_DEFINE(Effect);

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

// Reads an optional (string | nil) hash field into a std::optional<string>.
static void GetOptionalString(mrb_state* mrb,
                              mrb_value hash,
                              const char* key,
                              std::optional<std::string>& out) {
  mrb_value val =
      mrb_hash_get(mrb, hash, mrb_symbol_value(mrb_intern_cstr(mrb, key)));
  if (mrb_nil_p(val))
    return;
  if (!mrb_string_p(val))
    mrb_raise(mrb, E_TYPE_ERROR, "expected String");
  out = mrb_str_to_cstr(mrb, val);
}

// Parses an EffectCreateInfo struct from a Ruby Hash. Missing (or nil) keys
// keep the C++ struct defaults. Example:
//
//   Effect.new(vertex_shader: "...",
//              fragment_shader: "...",
//              color_blend: { src_rgb: 1, dst_rgb: 2, ... })
//
static lime::Effect::EffectCreateInfo GetEffectCreateInfo(mrb_state* mrb,
                                                          mrb_value hash) {
  lime::Effect::EffectCreateInfo info;
  if (!mrb_hash_p(hash))
    mrb_raise(mrb, E_TYPE_ERROR, "expected Hash");

  GetOptionalString(mrb, hash, "vertex_shader", info.vertex_shader);
  GetOptionalString(mrb, hash, "fragment_shader", info.fragment_shader);

  mrb_value cb = mrb_hash_get(
      mrb, hash, mrb_symbol_value(mrb_intern_lit(mrb, "color_blend")));
  if (mrb_nil_p(cb))
    return info;
  if (!mrb_hash_p(cb))
    mrb_raise(mrb, E_TYPE_ERROR, "color_blend must be a Hash");

  lime::Effect::ColorBlendState state;
  struct Field {
    const char* name;
    int* dst;
  } fields[] = {
      {"src_rgb", &state.src_rgb},     {"dst_rgb", &state.dst_rgb},
      {"src_alpha", &state.src_alpha}, {"dst_alpha", &state.dst_alpha},
      {"equal_rgb", &state.equal_rgb}, {"equal_alpha", &state.equal_alpha},
  };
  for (auto& field : fields) {
    mrb_value val = mrb_hash_get(
        mrb, cb, mrb_symbol_value(mrb_intern_cstr(mrb, field.name)));
    if (mrb_nil_p(val))
      continue;
    *field.dst = static_cast<int>(mrb_as_int(mrb, val));
  }
  info.color_blend = state;

  return info;
}

MRB_FUNC(Effect_initialize) {
  mrb_value create_info_val;
  mrb_get_args(mrb, "o", &create_info_val);

  lime::RefPtr<lime::Effect> obj = nullptr;
  EXC_BEGIN {
    auto create_info = GetEffectCreateInfo(mrb, create_info_val);
    obj = lime::MakeRefCounted<lime::Effect>(create_info);
  }
  EXC_END(mrb);

  return SetupSelfData(self, obj.get(), kEffectDataType);
}

MRB_FUNC(Effect_initialize_copy) {
  mrb_value other;
  mrb_get_args(mrb, "o", &other);

  lime::RefPtr<lime::Effect> obj = nullptr;
  EXC_BEGIN {
    auto other_obj = GetObject<lime::Effect>(mrb, other, kEffectDataType);
    obj = lime::MakeRefCounted<lime::Effect>(other_obj);
  }
  EXC_END(mrb);

  return SetupSelfData(self, obj.get(), kEffectDataType);
}

MRB_FUNC(Effect_SetValueF) {
  auto* self_obj = GetSelfData<lime::Effect>(self);

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

MRB_FUNC(Effect_SetValueI) {
  auto* self_obj = GetSelfData<lime::Effect>(self);

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

MRB_FUNC(Effect_SetValueU) {
  auto* self_obj = GetSelfData<lime::Effect>(self);

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

MRB_FUNC(Effect_SetValueT) {
  auto* self_obj = GetSelfData<lime::Effect>(self);
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

MRB_FUNC(Effect_SetValueM) {
  auto* self_obj = GetSelfData<lime::Effect>(self);
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

void InitEffectBinding(mrb_state* mrb) {
  auto klass = DefineClass(mrb, "Effect");

  mrb_define_method(mrb, klass, "initialize", Effect_initialize,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "initialize_copy", Effect_initialize_copy,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "set_value_f", Effect_SetValueF,
                    MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "set_value_i", Effect_SetValueI,
                    MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "set_value_u", Effect_SetValueU,
                    MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "set_bitmap", Effect_SetValueT,
                    MRB_ARGS_REQ(2));
  mrb_define_method(mrb, klass, "set_matrix", Effect_SetValueM,
                    MRB_ARGS_REQ(2));

  // OpenGL blend constants (use with color_blend.src_rgb/dst_rgb/src_alpha/
  // dst_alpha and color_blend.equal_rgb/equal_alpha in Effect.new)
  struct BlendConstant {
    const char* name;
    int value;
  } blend_constants[] = {
      // Blend factors
      {"FACTOR_ZERO", RL_ZERO},
      {"FACTOR_ONE", RL_ONE},
      {"FACTOR_SRC_COLOR", RL_SRC_COLOR},
      {"FACTOR_ONE_MINUS_SRC_COLOR", RL_ONE_MINUS_SRC_COLOR},
      {"FACTOR_SRC_ALPHA", RL_SRC_ALPHA},
      {"FACTOR_ONE_MINUS_SRC_ALPHA", RL_ONE_MINUS_SRC_ALPHA},
      {"FACTOR_DST_ALPHA", RL_DST_ALPHA},
      {"FACTOR_ONE_MINUS_DST_ALPHA", RL_ONE_MINUS_DST_ALPHA},
      {"FACTOR_DST_COLOR", RL_DST_COLOR},
      {"FACTOR_ONE_MINUS_DST_COLOR", RL_ONE_MINUS_DST_COLOR},
      {"FACTOR_SRC_ALPHA_SATURATE", RL_SRC_ALPHA_SATURATE},
      {"FACTOR_CONSTANT_COLOR", RL_CONSTANT_COLOR},
      {"FACTOR_ONE_MINUS_CONSTANT_COLOR", RL_ONE_MINUS_CONSTANT_COLOR},
      {"FACTOR_CONSTANT_ALPHA", RL_CONSTANT_ALPHA},
      {"FACTOR_ONE_MINUS_CONSTANT_ALPHA", RL_ONE_MINUS_CONSTANT_ALPHA},
      // Blend equations
      {"EQUATION_ADD", RL_FUNC_ADD},
      {"EQUATION_MIN", RL_MIN},
      {"EQUATION_MAX", RL_MAX},
      {"EQUATION_SUBTRACT", RL_FUNC_SUBTRACT},
      {"EQUATION_REVERSE_SUBTRACT", RL_FUNC_REVERSE_SUBTRACT},
  };
  for (auto& constant : blend_constants) {
    mrb_const_set(mrb, mrb_obj_value(klass),
                  mrb_intern_cstr(mrb, constant.name),
                  mrb_fixnum_value(constant.value));
  }
}

}  // namespace binding
