#pragma once

#include "common.h"

namespace rgssx {

struct ShaderBase {
  ~ShaderBase();

  raylib::Shader shader = {};
};

struct SpriteShader : public ShaderBase {
  SpriteShader();

  int u_color = 0;
  int u_tone = 0;
  int u_opacity = 0;
  int u_bush_depth = 0;
  int u_bush_opacity = 0;
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

struct ViewportShader : public ShaderBase {
  ViewportShader();

  int u_color = 0;
  int u_tone = 0;
  int u_opacity = 0;
};

struct TilemapShader : public ShaderBase {
  TilemapShader();

  int u_offset = 0;
  int u_anim_offset = 0;
  int u_tile_size = 0;
  int u_flash_alpha = 0;
};

struct ShaderSet : public Singleton<ShaderSet> {
  ShaderSet() = default;

  SpriteShader sprite;
  AlphaTransition alpha_trans;
  MappingTransition mapping_trans;
  ViewportShader viewport;
  TilemapShader tilemap;
};

}  // namespace rgssx
