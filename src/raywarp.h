// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Admenri Adev <admenri0504@gmail.com>.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
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

/// --------------------------------------------------------------
///  Raylib draw screen projection hook
/// --------------------------------------------------------------
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

/// --------------------------------------------------------------
///  Raylib scissor helper
/// --------------------------------------------------------------

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

/// --------------------------------------------------------------
///  Raylib color helper
/// --------------------------------------------------------------
template <typename Ty>
Color MakeColor(Ty value) {
  Color result = {};
  result.r = static_cast<uint8_t>(value);
  result.g = static_cast<uint8_t>(value);
  result.b = static_cast<uint8_t>(value);
  result.a = static_cast<uint8_t>(value);
  return result;
}

/// --------------------------------------------------------------
///  Raylib rectangle helper
/// --------------------------------------------------------------
inline Rectangle IntRect(int x, int y, int w, int h) {
  Rectangle rect = {};
  rect.x = static_cast<float>(x);
  rect.y = static_cast<float>(y);
  rect.width = static_cast<float>(w);
  rect.height = static_cast<float>(h);
  return rect;
}

/// --------------------------------------------------------------
///  RGSS blend type (PMA)
/// --------------------------------------------------------------
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

/// --------------------------------------------------------------
///  Raylib draw helper
/// --------------------------------------------------------------
inline void DrawTextureTiled(Texture2D texture,
                             Rectangle source,
                             Rectangle dest,
                             Vector2 origin,
                             float rotation,
                             float scale,
                             Color tint) {
  if ((texture.id <= 0) || (scale <= 0.0f))
    return;  // Wanna see a infinite loop?!...just delete this line!
  if ((source.width == 0) || (source.height == 0))
    return;

  int tileWidth = (int)(source.width * scale),
      tileHeight = (int)(source.height * scale);
  if ((dest.width < tileWidth) && (dest.height < tileHeight)) {
    // Can fit only one tile
    DrawTexturePro(texture,
                   Rectangle{source.x, source.y,
                             ((float)dest.width / tileWidth) * source.width,
                             ((float)dest.height / tileHeight) * source.height},
                   Rectangle{dest.x, dest.y, dest.width, dest.height}, origin,
                   rotation, tint);
  } else if (dest.width <= tileWidth) {
    // Tiled vertically (one column)
    int dy = 0;
    for (; dy + tileHeight < dest.height; dy += tileHeight) {
      DrawTexturePro(
          texture,
          Rectangle{source.x, source.y,
                    ((float)dest.width / tileWidth) * source.width,
                    source.height},
          Rectangle{dest.x, dest.y + dy, dest.width, (float)tileHeight}, origin,
          rotation, tint);
    }

    // Fit last tile
    if (dy < dest.height) {
      DrawTexturePro(
          texture,
          Rectangle{source.x, source.y,
                    ((float)dest.width / tileWidth) * source.width,
                    ((float)(dest.height - dy) / tileHeight) * source.height},
          Rectangle{dest.x, dest.y + dy, dest.width, dest.height - dy}, origin,
          rotation, tint);
    }
  } else if (dest.height <= tileHeight) {
    // Tiled horizontally (one row)
    int dx = 0;
    for (; dx + tileWidth < dest.width; dx += tileWidth) {
      DrawTexturePro(
          texture,
          Rectangle{source.x, source.y, source.width,
                    ((float)dest.height / tileHeight) * source.height},
          Rectangle{dest.x + dx, dest.y, (float)tileWidth, dest.height}, origin,
          rotation, tint);
    }

    // Fit last tile
    if (dx < dest.width) {
      DrawTexturePro(
          texture,
          Rectangle{source.x, source.y,
                    ((float)(dest.width - dx) / tileWidth) * source.width,
                    ((float)dest.height / tileHeight) * source.height},
          Rectangle{dest.x + dx, dest.y, dest.width - dx, dest.height}, origin,
          rotation, tint);
    }
  } else {
    // Tiled both horizontally and vertically (rows and columns)
    int dx = 0;
    for (; dx + tileWidth < dest.width; dx += tileWidth) {
      int dy = 0;
      for (; dy + tileHeight < dest.height; dy += tileHeight) {
        DrawTexturePro(texture, source,
                       Rectangle{dest.x + dx, dest.y + dy, (float)tileWidth,
                                 (float)tileHeight},
                       origin, rotation, tint);
      }

      if (dy < dest.height) {
        DrawTexturePro(
            texture,
            Rectangle{source.x, source.y, source.width,
                      ((float)(dest.height - dy) / tileHeight) * source.height},
            Rectangle{dest.x + dx, dest.y + dy, (float)tileWidth,
                      dest.height - dy},
            origin, rotation, tint);
      }
    }

    // Fit last column of tiles
    if (dx < dest.width) {
      int dy = 0;
      for (; dy + tileHeight < dest.height; dy += tileHeight) {
        DrawTexturePro(
            texture,
            Rectangle{source.x, source.y,
                      ((float)(dest.width - dx) / tileWidth) * source.width,
                      source.height},
            Rectangle{dest.x + dx, dest.y + dy, dest.width - dx,
                      (float)tileHeight},
            origin, rotation, tint);
      }

      // Draw final tile in the bottom right corner
      if (dy < dest.height) {
        DrawTexturePro(
            texture,
            Rectangle{source.x, source.y,
                      ((float)(dest.width - dx) / tileWidth) * source.width,
                      ((float)(dest.height - dy) / tileHeight) * source.height},
            Rectangle{dest.x + dx, dest.y + dy, dest.width - dx,
                      dest.height - dy},
            origin, rotation, tint);
      }
    }
  }
}

}  // namespace raylib
