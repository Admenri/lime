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

  shader_ = raylib::LoadShaderFromMemory(vs_code, fs_code);
  color_blend_ = create_info.color_blend;

  if (!raylib::IsShaderValid(shader_))
    throw Exception(Exception::RGSSError, "failed to compile shader.");
}

Effect::~Effect() {
  raylib::UnloadShader(shader_);
}

void Effect::SetValueF(std::string uniform,
                       std::span<float> value,
                       int item_count) {
  int loc = GetValueLocation(uniform);
  raylib::SetShaderValueV(shader_, loc, value.data(),
                          raylib::SHADER_UNIFORM_FLOAT + item_count - 1,
                          value.size());
}

void Effect::SetValueI(std::string uniform,
                       std::span<int32_t> value,
                       int item_count) {
  int loc = GetValueLocation(uniform);
  raylib::SetShaderValueV(shader_, loc, value.data(),
                          raylib::SHADER_UNIFORM_INT + item_count - 1,
                          value.size());
}

void Effect::SetValueU(std::string uniform,
                       std::span<uint32_t> value,
                       int item_count) {
  int loc = GetValueLocation(uniform);
  raylib::SetShaderValueV(shader_, loc, value.data(),
                          raylib::SHADER_UNIFORM_UINT + item_count - 1,
                          value.size());
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

  raylib::SetShaderValueMatrix(shader_, loc, mat);
}

void Effect::BeginEffect() {
  raylib::rlSetShader(shader_.id, shader_.locs);

  for (auto& it : textures_) {
    if (it.second && !it.second->IsDisposed()) {
      auto& tex = it.second->render_texture();
      raylib::SetShaderValueTexture(shader_, it.first, tex.texture);
    }
  }

  if (color_blend_.has_value()) {
    raylib::rlEnableColorBlend();
    raylib::rlSetBlendFactorsSeparate(
        color_blend_->src_rgb, color_blend_->dst_rgb, color_blend_->src_alpha,
        color_blend_->dst_alpha, color_blend_->equal_rgb,
        color_blend_->equal_alpha);
  }
}

int Effect::GetValueLocation(std::string name) {
  auto it = locations_.find(name);
  if (it == locations_.end()) {
    int loc = raylib::GetShaderLocation(shader_, name.c_str());
    locations_[name] = loc;
    return loc;
  }

  return it->second;
}

}  // namespace lime
