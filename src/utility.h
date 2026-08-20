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

#include <algorithm>

#include "src/common.h"
#include "src/raywarp.h"
#include "src/refptr.h"

namespace lime {

class Rect : public RefCounted<Rect> {
 public:
  Rect(raylib::Rectangle rect)
      : x(static_cast<int>(rect.x)),
        y(static_cast<int>(rect.y)),
        width(static_cast<int>(rect.width)),
        height(static_cast<int>(rect.height)) {}

  /*-export.begin-*/
  Rect(int xv, int yv, int w, int h) : x(xv), y(yv), width(w), height(h) {}
  Rect(RefPtr<Rect> o) : x(o->x), y(o->y), width(o->width), height(o->height) {}
  Rect() {}

  MARSHAL_DUMP(Rect);
  MARSHAL_LOAD(Rect);

  void Set(int xv, int yv, int w, int h) {
    x = xv;
    y = yv;
    width = w;
    height = h;
  }

  void Set(RefPtr<Rect> rect) {
    if (rect) {
      x = rect->x;
      y = rect->y;
      width = rect->width;
      height = rect->height;
    } else {
      throw Exception(Exception::RGSSError, "cannot set null to rect.");
    }
  }

  void Empty() {
    x = 0;
    y = 0;
    width = 0;
    height = 0;
  }

  int x = 0, y = 0, width = 0, height = 0;
  /*-export.end-*/

 public:
  raylib::Rectangle As() {
    raylib::Rectangle result = {};
    result.x = static_cast<float>(x);
    result.y = static_cast<float>(y);
    result.width = static_cast<float>(width);
    result.height = static_cast<float>(height);
    return result;
  }
};

class Color : public RefCounted<Color> {
 public:
  Color(raylib::Color color)
      : red(static_cast<float>(color.r)),
        green(static_cast<float>(color.g)),
        blue(static_cast<float>(color.b)),
        alpha(static_cast<float>(color.a)) {}

  /*-export.begin-*/
  Color(float r, float g, float b, float a = 255.f)
      : red(r), green(g), blue(b), alpha(a) {}
  Color(RefPtr<Color> o)
      : red(o->red), green(o->green), blue(o->blue), alpha(o->alpha) {}
  Color() {}

  MARSHAL_DUMP(Color);
  MARSHAL_LOAD(Color);

  void Set(float r, float g, float b, float a = 255.f) {
    red = r;
    green = g;
    blue = b;
    alpha = a;
  }

  void Set(RefPtr<Color> color) {
    if (color) {
      red = color->red;
      green = color->green;
      blue = color->blue;
      alpha = color->alpha;
    } else {
      throw Exception(Exception::RGSSError, "cannot set null to color.");
    }
  }

  float red = 0, green = 0, blue = 0, alpha = 0;
  /*-export.end-*/

 public:
  raylib::Color As() {
    raylib::Color result = {};
    result.r = static_cast<uint8_t>(red);
    result.g = static_cast<uint8_t>(green);
    result.b = static_cast<uint8_t>(blue);
    result.a = static_cast<uint8_t>(alpha);
    return result;
  }

  raylib::Vector4 Normalize() {
    raylib::Vector4 vec = {};
    vec.x = red / 255.0f;
    vec.y = green / 255.0f;
    vec.z = blue / 255.0f;
    vec.w = alpha / 255.0f;
    return vec;
  }
};

class Tone : public RefCounted<Tone> {
 public:
  /*-export.begin-*/
  Tone(float r, float g, float b, float a = 0.f)
      : red(r), green(g), blue(b), gray(a) {}
  Tone(RefPtr<Tone> o)
      : red(o->red), green(o->green), blue(o->blue), gray(o->gray) {}
  Tone() {}

  MARSHAL_DUMP(Tone);
  MARSHAL_LOAD(Tone);

  void Set(float r, float g, float b, float a = 0.f) {
    red = r;
    green = g;
    blue = b;
    gray = a;
  }

  void Set(RefPtr<Tone> tone) {
    if (tone) {
      red = tone->red;
      green = tone->green;
      blue = tone->blue;
      gray = tone->gray;
    } else {
      throw Exception(Exception::RGSSError, "cannot set null to tone.");
    }
  }

  float red = 0, green = 0, blue = 0, gray = 0;
  /*-export.end-*/

 public:
  raylib::Vector4 Normalize() {
    raylib::Vector4 vec = {};
    vec.x = red / 255.0f;
    vec.y = green / 255.0f;
    vec.z = blue / 255.0f;
    vec.w = gray / 255.0f;
    return vec;
  }

  bool HasEffect() { return red || green || blue || gray; }
};

class Vector2 : public RefCounted<Vector2> {
 public:
  /*-export.begin-*/
  Vector2(float xv, float yv) : x(xv), y(yv) {}
  Vector2(RefPtr<Vector2> o) : x(o->x), y(o->y) {}
  Vector2() {}

  void Set(float xv, float yv) {
    x = xv;
    y = yv;
  }

  void Set(RefPtr<Vector2> v) {
    x = v->x;
    y = v->y;
  }

  float x = 0, y = 0;
  /*-export.end-*/

 public:
  raylib::Vector2 As() {
    raylib::Vector2 result = {};
    result.x = x;
    result.y = y;
    return result;
  }
};

class Vector3 : public RefCounted<Vector3> {
 public:
  /*-export.begin-*/
  Vector3(float xv, float yv, float zv) : x(xv), y(yv), z(zv) {}
  Vector3(RefPtr<Vector3> o) : x(o->x), y(o->y), z(o->z) {}
  Vector3() {}

  void Set(float xv, float yv, float zv) {
    x = xv;
    y = yv;
    z = zv;
  }

  void Set(RefPtr<Vector3> v) {
    x = v->x;
    y = v->y;
    z = v->z;
  }

  float x = 0, y = 0, z = 0;
  /*-export.end-*/

 public:
  raylib::Vector3 As() {
    raylib::Vector3 result = {};
    result.x = x;
    result.y = y;
    result.z = z;
    return result;
  }
};

class Vector4 : public RefCounted<Vector4> {
 public:
  /*-export.begin-*/
  Vector4(float xv, float yv, float zv, float wv)
      : x(xv), y(yv), z(zv), w(wv) {}
  Vector4(RefPtr<Vector4> o) : x(o->x), y(o->y), z(o->z), w(o->w) {}
  Vector4() {}

  void Set(float xv, float yv, float zv, float wv) {
    x = xv;
    y = yv;
    z = zv;
    w = wv;
  }

  void Set(RefPtr<Vector4> v) {
    x = v->x;
    y = v->y;
    z = v->z;
    w = v->w;
  }

  float x = 0, y = 0, z = 0, w = 0;
  /*-export.end-*/

 public:
  raylib::Vector4 As() {
    raylib::Vector4 result = {};
    result.x = x;
    result.y = y;
    result.z = z;
    result.w = w;
    return result;
  }
};

}  // namespace lime
