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

#include <span>
#include <unordered_map>
#include <vector>

#include "src/common.h"
#include "src/raywarp.h"
#include "src/refptr.h"

namespace lime {

class Bitmap;

class Effect : public RefCounted<Effect> {
 public:
  struct ColorBlendState {
    int src_rgb = RL_ONE;
    int dst_rgb = RL_ONE_MINUS_SRC_ALPHA;
    int src_alpha = RL_ONE;
    int dst_alpha = RL_ONE_MINUS_SRC_ALPHA;
    int equal_rgb = RL_FUNC_ADD;
    int equal_alpha = RL_FUNC_ADD;
  };

  struct EffectCreateInfo {
    std::optional<std::string> vertex_shader;
    std::optional<std::string> fragment_shader;
    std::optional<ColorBlendState> color_blend;
  };

  Effect(const EffectCreateInfo& create_info);
  Effect(RefPtr<Effect> other);
  ~Effect();

  /*-export.begin-*/
  void SetValueF(std::string uniform,
                 std::span<float> value,
                 int item_count = 1);
  void SetValueI(std::string uniform,
                 std::span<int32_t> value,
                 int item_count = 1);
  void SetValueU(std::string uniform,
                 std::span<uint32_t> value,
                 int item_count = 1);
  void SetValueT(std::string uniform, RefPtr<Bitmap> texture);
  void SetValueM(std::string uniform, float value[16]);
  /*-export.end-*/

 public:
  void BeginEffect();
  void EndEffect();

 private:
  int GetValueLocation(std::string name);

  struct ShaderWrapper : public RefCounted<ShaderWrapper> {
    raylib::Shader shader;

    ShaderWrapper(raylib::Shader s) : shader(s) {}
    ~ShaderWrapper() { raylib::UnloadShader(shader); }
  };

  template <typename T>
  struct UniformArrayValue {
    std::vector<T> values;
    int item_count = 1;
  };

  RefPtr<ShaderWrapper> shader_;
  std::optional<ColorBlendState> color_blend_;

  std::unordered_map<std::string, int> locations_;
  std::unordered_map<int, UniformArrayValue<float>> float_values_;
  std::unordered_map<int, UniformArrayValue<int32_t>> int_values_;
  std::unordered_map<int, UniformArrayValue<uint32_t>> uint_values_;
  std::unordered_map<int, raylib::Matrix> matrix_values_;
  std::unordered_map<int, RefPtr<Bitmap>> textures_;
};

}  // namespace lime
