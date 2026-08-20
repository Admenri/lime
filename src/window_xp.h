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

class WindowXP;

class WindowXPAbove : public ViewportChild {
 public:
  WindowXPAbove(WindowXP* parent, RefPtr<Viewport> viewport);

 private:
  void Draw(DrawParam param) override;

  WindowXP* parent_;
};

class WindowXP : public RefCounted<WindowXP>,
                 public Dispoable,
                 public ViewportChild {
 public:
  WindowXP(RefPtr<Viewport> viewport = nullptr);
  ~WindowXP();

  /*-export.begin-*/
  void Update();

  ATTR(RefPtr<Viewport>, Viewport) override;
  ATTR(bool, Visible) override;
  ATTR(int, Z) override;

  ATTR(RefPtr<Bitmap>, WindowSkin);
  ATTR(RefPtr<Bitmap>, Contents);
  ATTR(bool, Stretch);
  ATTR(RefPtr<Rect>, CursorRect);
  ATTR(bool, Active);
  ATTR(bool, Pause);
  ATTR(int, X);
  ATTR(int, Y);
  ATTR(int, Width);
  ATTR(int, Height);
  ATTR(int, OX);
  ATTR(int, OY);
  ATTR(int, Opacity);
  ATTR(int, BackOpacity);
  ATTR(int, ContentsOpacity);
  /*-export.end-*/

 private:
  friend class WindowXPAbove;
  void DisposeObject() override;
  void Draw(DrawParam param) override;

  void DrawGround(DrawParam param);
  void DrawAbove(DrawParam param);

  std::unique_ptr<WindowXPAbove> above_;
  int32_t scale_ = 2;
  int32_t pause_index_ = 0;
  int32_t cursor_opacity_ = 255;
  bool cursor_fade_ = false;

  RefPtr<Bitmap> windowskin_;
  RefPtr<Bitmap> contents_;
  bool stretch_ = true;
  RefPtr<Rect> cursor_rect_;
  bool active_ = true;
  bool pause_ = false;
  int x_ = 0, y_ = 0, width_ = 0, height_ = 0;
  int ox_ = 0, oy_ = 0;
  int opacity_ = 255, back_opacity_ = 255, contents_opacity_ = 255;
};

}  // namespace lime
