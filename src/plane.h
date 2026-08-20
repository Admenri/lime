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

#include "src/bitmap.h"
#include "src/common.h"
#include "src/drawable.h"
#include "src/refptr.h"
#include "src/viewport.h"

namespace lime {

class Plane : public RefCounted<Plane>, public Dispoable, public ViewportChild {
 public:
  /*-export.begin-*/
  Plane(RefPtr<Viewport> viewport = nullptr);
  ~Plane();

  ATTR(RefPtr<Bitmap>, Bitmap);
  ATTR(int, OX);
  ATTR(int, OY);
  ATTR(float, ZoomX);
  ATTR(float, ZoomY);
  ATTR(int, Opacity);
  ATTR(int, BlendType);
  ATTR(RefPtr<Color>, Color);
  ATTR(RefPtr<Tone>, Tone);
  /*-export.end-*/

 private:
  void DisposeObject() override;
  void Draw(DrawParam param) override;

  RefPtr<Bitmap> bitmap_;
  int ox_ = 0, oy_ = 0;
  float zoom_x_ = 1.0f, zoom_y_ = 1.0f;
  int opacity_ = 255, blend_type_ = 0;
  RefPtr<Color> color_;
  RefPtr<Tone> tone_;
};

}  // namespace lime
