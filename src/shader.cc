#include "src/shader.h"

#include "src/bitmap.h"

namespace lime {

Shader::Shader(std::string vs_code, std::string fs_code) {
  shader_ =
      raylib::LoadShaderFromMemory(vs_code.empty() ? nullptr : vs_code.c_str(),
                                   fs_code.empty() ? nullptr : fs_code.c_str());
  if (!raylib::IsShaderValid(shader_))
    throw Exception(Exception::RGSSError, "failed to compile shader.");
}

Shader::~Shader() {
  raylib::UnloadShader(shader_);
}

void Shader::SetValueF(std::string uniform,
                       std::span<float> value,
                       int item_count) {
  int loc = GetValueLocation(uniform);
  raylib::SetShaderValueV(shader_, loc, value.data(),
                          raylib::SHADER_UNIFORM_FLOAT + item_count - 1,
                          value.size());
}

void Shader::SetValueI(std::string uniform,
                       std::span<int32_t> value,
                       int item_count) {
  int loc = GetValueLocation(uniform);
  raylib::SetShaderValueV(shader_, loc, value.data(),
                          raylib::SHADER_UNIFORM_INT + item_count - 1,
                          value.size());
}

void Shader::SetValueU(std::string uniform,
                       std::span<uint32_t> value,
                       int item_count) {
  int loc = GetValueLocation(uniform);
  raylib::SetShaderValueV(shader_, loc, value.data(),
                          raylib::SHADER_UNIFORM_UINT + item_count - 1,
                          value.size());
}

void Shader::SetValueT(std::string uniform, RefPtr<Bitmap> texture) {
  int loc = GetValueLocation(uniform);

  if (texture && !texture->IsDisposed())
    textures_[loc] = texture;
  else
    textures_.erase(loc);
}

void Shader::SetValueM(std::string uniform, float value[16]) {
  int loc = GetValueLocation(uniform);

  raylib::Matrix mat = {};
  std::memcpy(&mat, value, sizeof(float) * 16);

  raylib::SetShaderValueMatrix(shader_, loc, mat);
}

void Shader::BeginEffect() {
  raylib::rlSetShader(shader_.id, shader_.locs);

  for (auto& it : textures_) {
    if (it.second && !it.second->IsDisposed()) {
      auto& tex = it.second->render_texture();
      raylib::SetShaderValueTexture(shader_, it.first, tex.texture);
    }
  }
}

int Shader::GetValueLocation(std::string name) {
  auto it = locations_.find(name);
  if (it == locations_.end()) {
    int loc = raylib::GetShaderLocation(shader_, name.c_str());
    locations_[name] = loc;
    return loc;
  }

  return it->second;
}

}  // namespace lime
