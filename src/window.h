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
#include "src/utility.h"
#include "src/viewport.h"

namespace lime {

class Window : public RefCounted<Window>,
               public Dispoable,
               public ViewportChild {
 public:
  Window(int x, int y, int width, int height);
  Window();
  ~Window();

  /*-export.begin-*/
  void Update();
  void Move(int x, int y, int width, int height);
  bool Opened();
  bool Closed();

  ATTR(RefPtr<Bitmap>, WindowSkin);
  ATTR(RefPtr<Bitmap>, Contents);
  ATTR(RefPtr<Rect>, CursorRect);
  ATTR(bool, Active);
  ATTR(bool, ArrowsVisible);
  ATTR(bool, Pause);
  ATTR(int, X);
  ATTR(int, Y);
  ATTR(int, Width);
  ATTR(int, Height);
  ATTR(int, OX);
  ATTR(int, OY);
  ATTR(int, Padding);
  ATTR(int, PaddingBottom);
  ATTR(int, Opacity);
  ATTR(int, BackOpacity);
  ATTR(int, ContentsOpacity);
  ATTR(int, Openness);
  ATTR(RefPtr<Tone>, Tone);
  ATTR(int, Scale);
  /*-export.end-*/

 private:
  void DisposeObject() override;
  void Draw(DrawParam param) override;

  bool rgss3_style_ = true;

  RefPtr<Bitmap> window_skin_;
  RefPtr<Bitmap> contents_;
  RefPtr<Rect> cursor_rect_;
  bool active_ = true, arrows_visible_ = true, pause_ = false;
  int x_ = 0, y_ = 0, width_ = 0, height_ = 0;
  int ox_ = 0, oy_ = 0;
  int padding_ = 12, padding_bottom_ = 12;
  int opacity_ = 255, back_opacity_ = 192, contents_opacity_ = 255;
  int openness_ = 255;
  RefPtr<Tone> tone_;
  int scale_ = 2;

  int pause_index_ = 0;
  int cursor_index_ = 0;
};

}  // namespace lime
