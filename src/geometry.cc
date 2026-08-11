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

  data_[triangle].color[point] = color->As();
}

ATTR_DEF(int, Capacity, Geometry) {
  if (value.has_value()) {
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

ATTR_DEF(RefPtr<Shader>, Shader, Geometry) {
  if (value.has_value()) {
    shader_ = *value;
    return std::nullopt;
  } else {
    return shader_;
  }
}

void Geometry::DisposeObject() {
  Drawable::RemoveFromList();

  bitmap_.reset();
  shader_.reset();
}

void Geometry::Draw(DrawParam param) {
  if (shader_)
    shader_->BeginEffect();
  {
    raylib::rlEnableColorBlend();
    raylib::rlSetBlendMode(raylib::GetBlendID(blend_type_));

    for (auto& triangle : data_) {
      if (bitmap_ && !bitmap_->IsDisposed())
        raylib::rlSetTexture(bitmap_->render_texture().texture.id);

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
  }
  if (shader_)
    raylib::EndShaderMode();
}

}  // namespace lime
