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

#include "src/bitmap.h"
#include "src/common.h"
#include "src/drawable.h"
#include "src/effect.h"
#include "src/refptr.h"
#include "src/viewport.h"

namespace lime {

class Sprite : public RefCounted<Sprite>,
               public Dispoable,
               public ViewportChild {
 public:
  Sprite(RefPtr<Viewport> viewport = nullptr);
  ~Sprite();

  /*-export.begin-*/
  void Flash(RefPtr<Color> color, int duration);
  void Update();

  int Width();
  int Height();

  ATTR(RefPtr<Bitmap>, Bitmap);
  ATTR(RefPtr<Rect>, SrcRect);
  ATTR(int, X);
  ATTR(int, Y);
  ATTR(int, OX);
  ATTR(int, OY);
  ATTR(float, ZoomX);
  ATTR(float, ZoomY);
  ATTR(float, Angle);
  ATTR(int, WaveAmp);
  ATTR(int, WaveLength);
  ATTR(int, WaveSpeed);
  ATTR(float, WavePhase);
  ATTR(bool, Mirror);
  ATTR(int, BushDepth);
  ATTR(int, BushOpacity);
  ATTR(int, Opacity);
  ATTR(int, BlendType);
  ATTR(RefPtr<Color>, Color);
  ATTR(RefPtr<Tone>, Tone);
  ATTR(RefPtr<Effect>, Effect);
  /*-export.end-*/

 private:
  void DisposeObject() override;
  void Draw(DrawParam param) override;

  RefPtr<Bitmap> bitmap_;
  RefPtr<Rect> src_rect_;
  int x_ = 0, y_ = 0;
  int ox_ = 0, oy_ = 0;
  float zoom_x_ = 1.0f, zoom_y_ = 1.0f;
  float angle_ = 0.0f;
  int wave_amp_ = 0, wave_length_ = 180, wave_speed_ = 360;
  float wave_phase_ = 0.0f;
  bool mirror_ = false;
  int bush_depth_ = 0, bush_opacity_ = 128;
  int opacity_ = 255, blend_type_ = 0;
  RefPtr<Color> color_;
  RefPtr<Tone> tone_;
  RefPtr<Effect> effect_;

  struct {
    raylib::Vector4 color = {};
    float step = 0.0f;
  } flash_;
};

}  // namespace lime
