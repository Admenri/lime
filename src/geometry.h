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
#include "src/refptr.h"
#include "src/utility.h"
#include "src/viewport.h"

namespace lime {

class Geometry : public RefCounted<Geometry>,
                 public Dispoable,
                 public ViewportChild {
 public:
  Geometry(RefPtr<Viewport> viewport = nullptr);
  ~Geometry();

  /*-export.begin-*/
  void SetPosition(int triangle, int point, RefPtr<Vector3> position);
  void SetTexcoord(int triangle, int point, RefPtr<Vector2> texcoord);
  void SetColor(int triangle, int point, RefPtr<Color> color);

  ATTR(int, Capacity);
  ATTR(RefPtr<Bitmap>, Bitmap);
  ATTR(int, BlendType);
  ATTR(RefPtr<Effect>, Effect);
  /*-export.end-*/

 private:
  void DisposeObject() override;
  void Draw(DrawParam param) override;

  struct TriangleData {
    raylib::Vector3 position[3] = {};
    raylib::Vector2 texcoord[3] = {};
    raylib::Color color[3] = {};
  };

  RefPtr<Bitmap> bitmap_;
  int blend_type_ = 0;
  RefPtr<Effect> effect_;

  std::vector<TriangleData> data_;
};

}  // namespace lime
