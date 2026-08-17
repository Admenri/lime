#include "src/sprite.h"

#include "src/shader.h"

namespace lime {

Sprite::Sprite(RefPtr<Viewport> viewport)
    : ViewportChild(viewport, ZValue()),
      src_rect_(MakeRefCounted<Rect>()),
      color_(MakeRefCounted<Color>()),
      tone_(MakeRefCounted<Tone>()) {}

Sprite::~Sprite() {
  Dispose();
}

void Sprite::Flash(RefPtr<Color> color, int duration) {
  flash_.color = color ? color->Normalize() : raylib::Vector4{};
  flash_.step =
      duration > 0 ? (flash_.color.w / static_cast<float>(duration)) : 0.0f;
}

void Sprite::Update() {
  wave_phase_ += wave_speed_ / 180.0f;
  wave_phase_ = std::fmod(wave_phase_, 360.0f);

  flash_.color.w -= flash_.step;
  if (flash_.color.w <= 0) {
    flash_.color = {};
    flash_.step = 0.0f;
  }
}

int Sprite::Width() {
  return src_rect_->width;
}

int Sprite::Height() {
  return src_rect_->height;
}

ATTR_DEF(RefPtr<Bitmap>, Bitmap, Sprite) {
  if (value.has_value()) {
    bitmap_ = *value;
    if (src_rect_ && bitmap_)
      src_rect_->Set(0, 0, bitmap_->Width(), bitmap_->Height());
    return std::nullopt;
  } else {
    return bitmap_;
  }
}

ATTR_DEF(RefPtr<Rect>, SrcRect, Sprite) {
  if (value.has_value()) {
    src_rect_ = *value;
    return std::nullopt;
  } else {
    return src_rect_;
  }
}

ATTR_DEF(int, X, Sprite) {
  if (value.has_value()) {
    x_ = *value;
    return std::nullopt;
  } else {
    return x_;
  }
}

ATTR_DEF(int, Y, Sprite) {
  if (value.has_value()) {
    y_ = *value;
    // RGSS behavior
    ZValue old_z = order();
    order().sorting = *value;
    Drawable::Resort(old_z);
    return std::nullopt;
  } else {
    return y_;
  }
}

ATTR_DEF(int, OX, Sprite) {
  if (value.has_value()) {
    ox_ = *value;
    return std::nullopt;
  } else {
    return ox_;
  }
}

ATTR_DEF(int, OY, Sprite) {
  if (value.has_value()) {
    oy_ = *value;
    return std::nullopt;
  } else {
    return oy_;
  }
}

ATTR_DEF(float, ZoomX, Sprite) {
  if (value.has_value()) {
    zoom_x_ = *value;
    return std::nullopt;
  } else {
    return zoom_x_;
  }
}

ATTR_DEF(float, ZoomY, Sprite) {
  if (value.has_value()) {
    zoom_y_ = *value;
    return std::nullopt;
  } else {
    return zoom_y_;
  }
}

ATTR_DEF(float, Angle, Sprite) {
  if (value.has_value()) {
    angle_ = *value;
    return std::nullopt;
  } else {
    return angle_;
  }
}

ATTR_DEF(int, WaveAmp, Sprite) {
  if (value.has_value()) {
    wave_amp_ = *value;
    return std::nullopt;
  } else {
    return wave_amp_;
  }
}

ATTR_DEF(int, WaveLength, Sprite) {
  if (value.has_value()) {
    wave_length_ = *value;
    return std::nullopt;
  } else {
    return wave_length_;
  }
}

ATTR_DEF(int, WaveSpeed, Sprite) {
  if (value.has_value()) {
    wave_speed_ = *value;
    return std::nullopt;
  } else {
    return wave_speed_;
  }
}

ATTR_DEF(float, WavePhase, Sprite) {
  if (value.has_value()) {
    wave_phase_ = *value;
    return std::nullopt;
  } else {
    return wave_phase_;
  }
}

ATTR_DEF(bool, Mirror, Sprite) {
  if (value.has_value()) {
    mirror_ = *value;
    return std::nullopt;
  } else {
    return mirror_;
  }
}

ATTR_DEF(int, BushDepth, Sprite) {
  if (value.has_value()) {
    bush_depth_ = *value;
    return std::nullopt;
  } else {
    return bush_depth_;
  }
}

ATTR_DEF(int, BushOpacity, Sprite) {
  if (value.has_value()) {
    bush_opacity_ = std::clamp<int>(*value, 0, 255);
    return std::nullopt;
  } else {
    return bush_opacity_;
  }
}

ATTR_DEF(int, Opacity, Sprite) {
  if (value.has_value()) {
    opacity_ = std::clamp<int>(*value, 0, 255);
    return std::nullopt;
  } else {
    return opacity_;
  }
}

ATTR_DEF(int, BlendType, Sprite) {
  if (value.has_value()) {
    blend_type_ = *value;
    return std::nullopt;
  } else {
    return blend_type_;
  }
}

ATTR_DEF(RefPtr<Color>, Color, Sprite) {
  if (value.has_value()) {
    if (!*value)
      throw Exception(Exception::RGSSError, "invalid value.");

    color_->Set(*value);
    return std::nullopt;
  } else {
    return color_;
  }
}

ATTR_DEF(RefPtr<Tone>, Tone, Sprite) {
  if (value.has_value()) {
    if (!*value)
      throw Exception(Exception::RGSSError, "invalid value.");

    tone_->Set(*value);
    return std::nullopt;
  } else {
    return tone_;
  }
}

ATTR_DEF(RefPtr<Effect>, Effect, Sprite) {
  if (value.has_value()) {
    effect_ = *value;
    return std::nullopt;
  } else {
    return effect_;
  }
}

void Sprite::DisposeObject() {
  Drawable::RemoveFromList();

  bitmap_.reset();
  effect_.reset();
}

void Sprite::Draw(DrawParam param) {
  if (bitmap_ && !bitmap_->IsDisposed()) {
    auto& bitmap_texture = bitmap_->render_texture();
    auto src_rectangle = src_rect_->As();
    if (mirror_)
      src_rectangle.width = -src_rectangle.width;

    auto& default_shader = ShaderSet::Instance()->sprite;
    if (effect_) {
      effect_->BeginEffect();
    } else {
      raylib::BeginShaderMode(default_shader.shader);
      raylib::rlEnableColorBlend();
      raylib::rlSetBlendMode(raylib::GetBlendID(blend_type_));
    }

    {
      if (!effect_) {
        // Default shader params
        auto color_norm = color_->Normalize();
        auto tone_norm = tone_->Normalize();
        auto opacity_norm = opacity_ / 255.0f;
        auto bush_depth_norm =
            (src_rectangle.y + src_rectangle.height - bush_depth_) /
            static_cast<float>(bitmap_texture.texture.height);
        auto bush_opacity_norm = bush_opacity_ / 255.0f;

        if (flash_.color.w > color_norm.w)
          color_norm = flash_.color;

        raylib::SetShaderValue(default_shader.shader, default_shader.u_color,
                               &color_norm, raylib::SHADER_UNIFORM_VEC4);
        raylib::SetShaderValue(default_shader.shader, default_shader.u_tone,
                               &tone_norm, raylib::SHADER_UNIFORM_VEC4);
        raylib::SetShaderValue(default_shader.shader, default_shader.u_opacity,
                               &opacity_norm, raylib::SHADER_UNIFORM_FLOAT);
        raylib::SetShaderValue(default_shader.shader,
                               default_shader.u_bush_depth, &bush_depth_norm,
                               raylib::SHADER_UNIFORM_FLOAT);
        raylib::SetShaderValue(
            default_shader.shader, default_shader.u_bush_opacity,
            &bush_opacity_norm, raylib::SHADER_UNIFORM_FLOAT);
      }

      raylib::rlMatrixMode(RL_MODELVIEW);
      raylib::rlPushMatrix();
      {
        raylib::rlTranslatef(x_, y_, 0.0f);
        raylib::rlRotatef(-angle_, 0.0f, 0.0f, 1.0f);
        raylib::rlScalef(zoom_x_, zoom_y_, 1.0f);
        raylib::rlTranslatef(-ox_, -oy_, 0.0f);

        if (wave_amp_ != 0 && wave_length_ != 0 && src_rect_->height > 0) {
          const int kSliceHeight = 8;
          int total_height = src_rect_->height;
          float src_x = static_cast<float>(src_rect_->x);
          float src_y = static_cast<float>(src_rect_->y);
          float src_w = static_cast<float>(src_rect_->width);
          float mirror_sign = mirror_ ? -1.0f : 1.0f;

          for (int offset_y = 0; offset_y < total_height;
               offset_y += kSliceHeight) {
            int slice_h = std::min(kSliceHeight, total_height - offset_y);

            float phase = (offset_y + wave_phase_) * 2.0f * PI / wave_length_;
            float wave_offset = wave_amp_ * sinf(phase);

            raylib::Rectangle src = {
                src_x, src_y + static_cast<float>(offset_y),
                src_w * mirror_sign, static_cast<float>(slice_h)};
            raylib::Rectangle dst = {wave_offset, static_cast<float>(offset_y),
                                     src_w, static_cast<float>(slice_h)};

            raylib::DrawTexturePro(bitmap_texture.texture, src, dst, {}, 0.0f,
                                   raylib::WHITE);
          }
        } else {
          raylib::DrawTextureRec(bitmap_texture.texture, src_rectangle, {}, {});
        }
      }
      raylib::rlPopMatrix();
    }

    if (effect_) {
      effect_->EndEffect();
    } else {
      raylib::EndShaderMode();
    }
  }
}

}  // namespace lime