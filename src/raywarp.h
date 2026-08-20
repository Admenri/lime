// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Admenri Adev <admenri0504@gmail.com>.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the “Software”), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <stdint.h>

namespace raylib {
#include "raylib.h"
#include "rlgl.h"

#include "3rdparty/raygui/src/raygui.h"

inline void BeginScreen() {
  BeginDrawing();

  rlMatrixMode(RL_PROJECTION);
  rlLoadIdentity();
  rlOrtho(0, GetRenderWidth(), 0, GetRenderHeight(), 0.0f, 1.0f);
}
inline void EndScreen() {
  EndDrawing();
}
#define BeginDrawing BeginScreen
#define EndDrawing EndScreen

inline Rectangle GetScissor() {
  int x, y, w, h;
  rlGetScissor(&x, &y, &w, &h);

  Rectangle result = {};
  result.x = static_cast<float>(x);
  result.y = static_cast<float>(y);
  result.width = static_cast<float>(w);
  result.height = static_cast<float>(h);
  return result;
}

inline void SetScissor(Rectangle rec) {
  raylib::rlScissor(static_cast<int>(rec.x), static_cast<int>(rec.y),
                    static_cast<int>(rec.width), static_cast<int>(rec.height));
}

inline void SetScissorTest(bool enable) {
  enable ? rlEnableScissorTest() : rlDisableScissorTest();
}

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

inline rlBlendMode GetBlendID(int type) {
  switch (type) {
    default:
    case 0:
      raylib::rlSetBlendFactorsSeparate(RL_ONE, RL_ONE_MINUS_SRC_ALPHA, RL_ONE,
                                        RL_ONE_MINUS_SRC_ALPHA, RL_FUNC_ADD,
                                        RL_FUNC_ADD);
      break;
    case 1:
      raylib::rlSetBlendFactorsSeparate(RL_ONE, RL_ONE, RL_ONE, RL_ONE,
                                        RL_FUNC_ADD, RL_FUNC_ADD);
      break;
    case 2:
      raylib::rlSetBlendFactorsSeparate(RL_ONE, RL_ONE, RL_ZERO, RL_ONE,
                                        RL_FUNC_REVERSE_SUBTRACT,
                                        RL_FUNC_REVERSE_SUBTRACT);
      break;
  }
  return raylib::RL_BLEND_CUSTOM_SEPARATE;
}

}  // namespace raylib
