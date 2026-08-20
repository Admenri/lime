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

#include "src/common.h"
#include "src/raywarp.h"
#include "src/utility.h"

namespace lime {

class DrawableSet;

struct DrawParam {
  raylib::Vector2 offset;
  raylib::RenderTexture2D target = {};
};

struct ZValue {
  int value = 0;
  int sorting = 0;
  double timestamp = 0.0;

  ZValue() : timestamp(raylib::GetTime()) {}
  ZValue(int v) : value(v), timestamp(raylib::GetTime()) {}
  ZValue(int v, int s) : value(v), sorting(s), timestamp(raylib::GetTime()) {}

  bool operator<(const ZValue& other) const {
    if (value != other.value)
      return value < other.value;
    if (sorting != other.sorting)
      return sorting < other.sorting;
    return timestamp < other.timestamp;
  }

  bool operator>(const ZValue& other) const { return other < *this; }
  bool operator<=(const ZValue& other) const { return !(*this > other); }
  bool operator>=(const ZValue& other) const { return !(*this < other); }
};

class Drawable {
 public:
  Drawable();
  Drawable(const ZValue& order);
  virtual ~Drawable();

  /*-export.begin-*/
  virtual ATTR(bool, Visible);
  virtual ATTR(int, Z);
  /*-export.end-*/

 public:
  virtual void Draw(DrawParam param) {}

 protected:
  ZValue& order() { return z_; }

  void Resort(ZValue old);
  void SetParent(DrawableSet* parent);
  void InsertAfter(Drawable* node);
  void RemoveFromList();

 private:
  friend class DrawableSet;

  void BubbleLeft();
  void BubbleRight();

  Drawable* prev_ = nullptr;
  Drawable* next_ = nullptr;

  DrawableSet* parent_ = nullptr;
  bool visible_ = true;
  ZValue z_ = {};
};

class DrawableSet {
 public:
  DrawableSet();
  ~DrawableSet();

  void DispatchDraw(DrawParam param);

 private:
  friend class Drawable;
  Drawable root_;  // sentinel node for doubly-linked list
};

}  // namespace lime
