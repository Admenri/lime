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

#include "src/geometry.h"

namespace lime {

Geometry::Geometry(RefPtr<Viewport> viewport)
    : ViewportChild(viewport, ZValue()) {}

Geometry::~Geometry() {
  Dispose();
}

void Geometry::SetPosition(int triangle, int point, RefPtr<Vector3> position) {
  Dispoable::Guard();

  if (triangle < 0 || triangle >= data_.size() || point < 0 || point >= 3)
    throw Exception(Exception::RGSSError, "invalid range");

  if (!position)
    throw Exception(Exception::RGSSError, "invalid data.");

  auto& v = data_[triangle];
  v.position[point].x = position->x;
  v.position[point].y = position->y;
  v.position[point].z = position->z;
}

void Geometry::SetTexcoord(int triangle, int point, RefPtr<Vector2> texcoord) {
  Dispoable::Guard();

  if (triangle < 0 || triangle >= data_.size() || point < 0 || point >= 3)
    throw Exception(Exception::RGSSError, "invalid range");

  if (!texcoord)
    throw Exception(Exception::RGSSError, "invalid data.");

  auto& v = data_[triangle];
  v.texcoord[point].x = texcoord->x;
  v.texcoord[point].y = texcoord->y;
}

void Geometry::SetColor(int triangle, int point, RefPtr<Color> color) {
  Dispoable::Guard();

  if (triangle < 0 || triangle >= data_.size() || point < 0 || point >= 3)
    throw Exception(Exception::RGSSError, "invalid range");

  if (!color)
    throw Exception(Exception::RGSSError, "invalid data.");

  data_[triangle].color[point] = color->As();
}

ATTR_DEF(int, Capacity, Geometry) {
  if (value.has_value()) {
    if (*value <= 0)
      throw Exception(Exception::RGSSError, "invalid capacity range.");

    data_.resize(*value);
    return std::nullopt;
  } else {
    return static_cast<int>(data_.size());
  }
}

ATTR_DEF(RefPtr<Bitmap>, Bitmap, Geometry) {
  if (value.has_value()) {
    bitmap_ = *value;
    return std::nullopt;
  } else {
    return bitmap_;
  }
}

ATTR_DEF(int, BlendType, Geometry) {
  if (value.has_value()) {
    blend_type_ = *value;
    return std::nullopt;
  } else {
    return blend_type_;
  }
}

ATTR_DEF(RefPtr<Effect>, Effect, Geometry) {
  if (value.has_value()) {
    effect_ = *value;
    return std::nullopt;
  } else {
    return effect_;
  }
}

void Geometry::DisposeObject() {
  Drawable::RemoveFromList();

  bitmap_.reset();
  effect_.reset();
}

void Geometry::Draw(DrawParam param) {
  if (effect_) {
    effect_->BeginEffect();
  } else {
    raylib::rlEnableColorBlend();
    raylib::rlSetBlendMode(raylib::GetBlendID(blend_type_));
  }

  for (auto& triangle : data_) {
    if (Dispoable::Check(bitmap_))
      raylib::rlSetTexture(bitmap_->handle().texture.id);
    else
      raylib::rlSetTexture(raylib::rlGetTextureIdDefault());

    raylib::rlBegin(RL_TRIANGLES);
    {
      raylib::rlNormal3f(0.0f, 0.0f, 1.0f);

      for (int i = 0; i < 3; ++i) {
        auto& color = triangle.color[i];
        auto& texcoord = triangle.texcoord[i];
        auto& position = triangle.position[i];

        raylib::rlColor4ub(color.r, color.g, color.b, color.a);
        raylib::rlTexCoord2f(texcoord.x, texcoord.y);
        raylib::rlVertex2f(position.x, position.y);
      }
    }
    raylib::rlEnd();
  }

  if (effect_)
    effect_->EndEffect();
}

}  // namespace lime
