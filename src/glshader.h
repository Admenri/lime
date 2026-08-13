#pragma once

#include "src/common.h"
#include "src/raywarp.h"

namespace lime {

struct ShaderBase {
  ShaderBase() = default;
  ~ShaderBase();

  ShaderBase(const ShaderBase&) = delete;
  ShaderBase& operator=(const ShaderBase&) = delete;

  raylib::Shader shader = {};
};

struct AlphaTransition : public ShaderBase {
  AlphaTransition();

  int u_frozen_image = 0;
  int u_progress = 0;
};

struct MappingTransition : public ShaderBase {
  MappingTransition();

  int u_frozen_image = 0;
  int u_mapping_image = 0;
  int u_progress = 0;
  int u_vague = 0;
};

struct SpriteShader : public ShaderBase {
  SpriteShader();

  int u_color = 0;
  int u_tone = 0;
  int u_opacity = 0;
  int u_bush_depth = 0;
  int u_bush_opacity = 0;
};

struct ViewportShader : public ShaderBase {
  ViewportShader();

  int u_color = 0;
  int u_tone = 0;
  int u_opacity = 0;
};

struct BitmapMaskShader : public ShaderBase {
  BitmapMaskShader();

  int u_mask = 0;
};

// --------------------------------------------------------------

struct ShaderSet : public Singleton<ShaderSet> {
  ShaderSet() = default;

  SpriteShader sprite;
  AlphaTransition alpha_trans;
  MappingTransition mapping_trans;
  ViewportShader viewport;
  BitmapMaskShader bitmap_mask;
};

}  // namespace lime
