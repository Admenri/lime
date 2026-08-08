#pragma once

#include <stdint.h>

namespace raylib {
#include "raylib.h"
#include "rlgl.h"

#include "3rdparty/raygui/src/raygui.h"

inline void BeginDrawTexture(RenderTexture2D target) {
  raylib::BeginTextureMode(target);

  raylib::rlMatrixMode(RL_PROJECTION);
  raylib::rlLoadIdentity();
  raylib::rlOrtho(0, target.texture.width, 0, target.texture.height, 0.0, 1.0);
}
inline void EndDrawTexture() {
  raylib::EndTextureMode();
}
#define BeginTextureMode BeginDrawTexture
#define EndTextureMode EndDrawTexture

template <typename Ty>
Color MakeColor(Ty value) {
  Color result = {};
  result.r = static_cast<uint8_t>(value);
  result.g = static_cast<uint8_t>(value);
  result.b = static_cast<uint8_t>(value);
  result.a = static_cast<uint8_t>(value);
  return result;
}

inline Rectangle IntRect(int x, int y, int w, int h) {
  Rectangle rect = {};
  rect.x = static_cast<float>(x);
  rect.y = static_cast<float>(y);
  rect.width = static_cast<float>(w);
  rect.height = static_cast<float>(h);
  return rect;
}

inline BlendMode GetBlendID(int type) {
  switch (type) {
    default:
    case 0:
      raylib::rlSetBlendFactorsSeparate(RL_ONE, RL_ONE_MINUS_SRC_ALPHA, RL_ONE,
                                        RL_ONE_MINUS_SRC_ALPHA, RL_FUNC_ADD,
                                        RL_FUNC_ADD);
      return raylib::BLEND_CUSTOM_SEPARATE;
    case 1:
      raylib::rlSetBlendFactorsSeparate(RL_ONE, RL_ONE, RL_ONE, RL_ONE,
                                        RL_FUNC_ADD, RL_FUNC_ADD);
      return raylib::BLEND_CUSTOM_SEPARATE;
    case 2:
      raylib::rlSetBlendFactorsSeparate(RL_ONE, RL_ONE, RL_ZERO, RL_ONE,
                                        RL_FUNC_REVERSE_SUBTRACT,
                                        RL_FUNC_REVERSE_SUBTRACT);
      return raylib::BLEND_CUSTOM_SEPARATE;
  }
}

}  // namespace raylib
