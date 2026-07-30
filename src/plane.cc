#include "plane.h"

#include <cmath>

#include "graphics.h"
#include "shader.h"

namespace rgssx {

Plane::Plane(RefPtr<Viewport> viewport)
    : ViewportChild(viewport),
      color_(MakeRefCounted<Color>()),
      tone_(MakeRefCounted<Tone>()) {}

Plane::~Plane() {
  Dispose();
}

ATTR_DEF(RefPtr<Bitmap>, Bitmap, Plane) {
  if (value.has_value()) {
    bitmap_ = *value;
    return std::nullopt;
  } else {
    return bitmap_;
  }
}

ATTR_DEF(int, OX, Plane) {
  if (value.has_value()) {
    ox_ = *value;
    return std::nullopt;
  } else {
    return ox_;
  }
}

ATTR_DEF(int, OY, Plane) {
  if (value.has_value()) {
    oy_ = *value;
    return std::nullopt;
  } else {
    return oy_;
  }
}

ATTR_DEF(float, ZoomX, Plane) {
  if (value.has_value()) {
    zoom_x_ = *value;
    return std::nullopt;
  } else {
    return zoom_x_;
  }
}

ATTR_DEF(float, ZoomY, Plane) {
  if (value.has_value()) {
    zoom_y_ = *value;
    return std::nullopt;
  } else {
    return zoom_y_;
  }
}

ATTR_DEF(int, Opacity, Plane) {
  if (value.has_value()) {
    opacity_ = std::clamp<int>(*value, 0, 255);
    return std::nullopt;
  } else {
    return opacity_;
  }
}

ATTR_DEF(int, BlendType, Plane) {
  if (value.has_value()) {
    blend_type_ = *value;
    return std::nullopt;
  } else {
    return blend_type_;
  }
}

ATTR_DEF(RefPtr<Color>, Color, Plane) {
  if (value.has_value()) {
    color_ = *value;
    return std::nullopt;
  } else {
    return color_;
  }
}

ATTR_DEF(RefPtr<Tone>, Tone, Plane) {
  if (value.has_value()) {
    tone_ = *value;
    return std::nullopt;
  } else {
    return tone_;
  }
}

void Plane::DisposeObject() {
  Drawable::RemoveFromList();
}

void Plane::Draw(DrawParam param) {
  if (bitmap_ && !bitmap_->IsDisposed()) {
    auto& bitmap_texture = bitmap_->render_texture();

    int bmpW = bitmap_->Width();
    int bmpH = bitmap_->Height();
    if (bmpW <= 0 || bmpH <= 0)
      return;

    float tileW = bmpW * zoom_x_;
    float tileH = bmpH * zoom_y_;

    auto& shader = ShaderSet::Instance()->viewport;

    raylib::BeginBlendMode(GetRaylibBlend(blend_type_));
    raylib::BeginShaderMode(shader.shader);
    {
      auto color_norm = color_->Normalize();
      auto tone_norm = tone_->Normalize();
      auto opacity_norm = opacity_ / 255.0f;

      raylib::SetShaderValue(shader.shader, shader.u_color, &color_norm,
                             raylib::SHADER_UNIFORM_VEC4);
      raylib::SetShaderValue(shader.shader, shader.u_tone, &tone_norm,
                             raylib::SHADER_UNIFORM_VEC4);
      raylib::SetShaderValue(shader.shader, shader.u_opacity, &opacity_norm,
                             raylib::SHADER_UNIFORM_FLOAT);

      // Compute tile grid origin: tile (0,0) starts at
      // (-ox_ * zoom_x_, -oy_ * zoom_y_) in viewport-local space
      float baseX = -ox_ * zoom_x_;
      float baseY = -oy_ * zoom_y_;

      auto& sc = param.scissor;

      // Calculate the range of tile indices that overlap the scissor area
      int firstCol = static_cast<int>(std::floor((sc.x - baseX) / tileW));
      int lastCol =
          static_cast<int>(std::ceil((sc.x + sc.width - baseX) / tileW));
      int firstRow = static_cast<int>(std::floor((sc.y - baseY) / tileH));
      int lastRow =
          static_cast<int>(std::ceil((sc.y + sc.height - baseY) / tileH));

      raylib::Rectangle src = {0.0f, 0.0f, static_cast<float>(bmpW),
                               static_cast<float>(bmpH)};

      for (int row = firstRow; row < lastRow; row++) {
        for (int col = firstCol; col < lastCol; col++) {
          raylib::Rectangle dst = {
              baseX + col * tileW,
              baseY + row * tileH,
              tileW,
              tileH,
          };

          raylib::DrawTexturePro(bitmap_texture.texture, src, dst, {0.0f, 0.0f},
                                 0.0f, raylib::WHITE);
        }
      }
    }
    raylib::EndShaderMode();
    raylib::EndBlendMode();
  }
}

}  // namespace rgssx