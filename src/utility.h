#pragma once

#include <algorithm>

#include "src/common.h"
#include "src/raywarp.h"
#include "src/refptr.h"

namespace rgssx {

class Rect : public RefCounted<Rect> {
 public:
  Rect(raylib::Rectangle rect)
      : x(static_cast<int>(rect.x)),
        y(static_cast<int>(rect.y)),
        width(static_cast<int>(rect.width)),
        height(static_cast<int>(rect.height)) {}
  Rect(int xv, int yv, int w, int h) : x(xv), y(yv), width(w), height(h) {}
  Rect(RefPtr<Rect> o) : x(o->x), y(o->y), width(o->width), height(o->height) {}
  Rect() {}

  /*-export.begin-*/
  MARSHAL_DUMP(Rect);
  MARSHAL_LOAD(Rect);

  void Set(int xv, int yv, int w, int h) {
    x = xv;
    y = yv;
    width = w;
    height = h;
  }

  void Set(RefPtr<Rect> rect) {
    x = rect->x;
    y = rect->y;
    width = rect->width;
    height = rect->height;
  }

  void Empty() {
    x = 0;
    y = 0;
    width = 0;
    height = 0;
  }

  int x = 0, y = 0, width = 0, height = 0;
  /*-export.end-*/

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
  Color(float r, float g, float b, float a = 255.f)
      : red(r), green(g), blue(b), alpha(a) {}
  Color(RefPtr<Color> o)
      : red(o->red), green(o->green), blue(o->blue), alpha(o->alpha) {}
  Color() {}

  /*-export.begin-*/
  MARSHAL_DUMP(Color);
  MARSHAL_LOAD(Color);

  void Set(float r, float g, float b, float a = 255.f) {
    red = r;
    green = g;
    blue = b;
    alpha = a;
  }

  void Set(RefPtr<Color> color) {
    red = color->red;
    green = color->green;
    blue = color->blue;
    alpha = color->alpha;
  }

  float red = 0, green = 0, blue = 0, alpha = 0;
  /*-export.end-*/

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
  Tone(float r, float g, float b, float a = 0.f)
      : red(r), green(g), blue(b), gray(a) {}
  Tone(RefPtr<Tone> o)
      : red(o->red), green(o->green), blue(o->blue), gray(o->gray) {}
  Tone() {}

  /*-export.begin-*/
  MARSHAL_DUMP(Tone);
  MARSHAL_LOAD(Tone);

  void Set(float r, float g, float b, float a = 0.f) {
    red = r;
    green = g;
    blue = b;
    gray = a;
  }

  void Set(RefPtr<Tone> tone) {
    red = tone->red;
    green = tone->green;
    blue = tone->blue;
    gray = tone->gray;
  }

  float red = 0, green = 0, blue = 0, gray = 0;
  /*-export.end-*/

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

}  // namespace rgssx
