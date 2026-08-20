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
