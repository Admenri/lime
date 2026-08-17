#include "src/effect.h"

#include "src/bitmap.h"

namespace lime {

Effect::Effect(const EffectCreateInfo& create_info) {
  auto* vs_code = create_info.vertex_shader.has_value()
                      ? create_info.vertex_shader->c_str()
                      : nullptr;
  auto* fs_code = create_info.fragment_shader.has_value()
                      ? create_info.fragment_shader->c_str()
                      : nullptr;

  shader_ = MakeRefCounted<ShaderWrapper>(
      raylib::LoadShaderFromMemory(vs_code, fs_code));
  color_blend_ = create_info.color_blend;

  if (!raylib::IsShaderValid(shader_->shader))
    throw Exception(Exception::RGSSError, "failed to compile shader.");
}

Effect::Effect(RefPtr<Effect> other)
    : shader_(other->shader_), color_blend_(other->color_blend_) {}

Effect::~Effect() = default;

void Effect::SetValueF(std::string uniform,
                       std::span<float> value,
                       int item_count) {
  int loc = GetValueLocation(uniform);
  auto& entry = float_values_[loc];
  entry.values.assign(value.begin(), value.end());
  entry.item_count = item_count;
}

void Effect::SetValueI(std::string uniform,
                       std::span<int32_t> value,
                       int item_count) {
  int loc = GetValueLocation(uniform);
  auto& entry = int_values_[loc];
  entry.values.assign(value.begin(), value.end());
  entry.item_count = item_count;
}

void Effect::SetValueU(std::string uniform,
                       std::span<uint32_t> value,
                       int item_count) {
  int loc = GetValueLocation(uniform);
  auto& entry = uint_values_[loc];
  entry.values.assign(value.begin(), value.end());
  entry.item_count = item_count;
}

void Effect::SetValueT(std::string uniform, RefPtr<Bitmap> texture) {
  int loc = GetValueLocation(uniform);

  if (texture && !texture->IsDisposed())
    textures_[loc] = texture;
  else
    textures_.erase(loc);
}

void Effect::SetValueM(std::string uniform, float value[16]) {
  int loc = GetValueLocation(uniform);

  raylib::Matrix mat = {};
  std::memcpy(&mat, value, sizeof(float) * 16);

  matrix_values_[loc] = mat;
}

void Effect::BeginEffect() {
  auto shader = shader_->shader;
  raylib::rlSetShader(shader.id, shader.locs);

  for (auto& it : float_values_) {
    raylib::SetShaderValueV(
        shader, it.first, it.second.values.data(),
        raylib::SHADER_UNIFORM_FLOAT + it.second.item_count - 1,
        it.second.values.size());
  }

  for (auto& it : int_values_) {
    raylib::SetShaderValueV(
        shader, it.first, it.second.values.data(),
        raylib::SHADER_UNIFORM_INT + it.second.item_count - 1,
        it.second.values.size());
  }

  for (auto& it : uint_values_) {
    raylib::SetShaderValueV(
        shader, it.first, it.second.values.data(),
        raylib::SHADER_UNIFORM_UINT + it.second.item_count - 1,
        it.second.values.size());
  }

  for (auto& it : matrix_values_) {
    raylib::SetShaderValueMatrix(shader, it.first, it.second);
  }

  for (auto& it : textures_) {
    if (it.second && !it.second->IsDisposed()) {
      auto& tex = it.second->render_texture();
      raylib::SetShaderValueTexture(shader, it.first, tex.texture);
    }
  }

  if (color_blend_.has_value()) {
    raylib::rlEnableColorBlend();
    raylib::rlSetBlendFactorsSeparate(
        color_blend_->src_rgb, color_blend_->dst_rgb, color_blend_->src_alpha,
        color_blend_->dst_alpha, color_blend_->equal_rgb,
        color_blend_->equal_alpha);
    raylib::rlSetBlendMode(raylib::RL_BLEND_CUSTOM_SEPARATE);
  } else {
    raylib::rlDisableColorBlend();
  }
}

void Effect::EndEffect() {
  raylib::rlSetShader(raylib::rlGetShaderIdDefault(),
                      raylib::rlGetShaderLocsDefault());
}

int Effect::GetValueLocation(std::string name) {
  auto it = locations_.find(name);
  if (it == locations_.end()) {
    int loc = raylib::GetShaderLocation(shader_->shader, name.c_str());
    locations_[name] = loc;
    return loc;
  }

  return it->second;
}

}  // namespace lime
