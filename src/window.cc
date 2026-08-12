#include "src/window.h"

#include "src/glshader.h"
#include "src/profile.h"

namespace raylib {

void DrawTextureTiled(Texture2D texture,
                      Rectangle source,
                      Rectangle dest,
                      Vector2 origin,
                      float rotation,
                      float scale,
                      Color tint) {
  if ((texture.id <= 0) || (scale <= 0.0f))
    return;  // Wanna see a infinite loop?!...just delete this line!
  if ((source.width == 0) || (source.height == 0))
    return;

  int tileWidth = (int)(source.width * scale),
      tileHeight = (int)(source.height * scale);
  if ((dest.width < tileWidth) && (dest.height < tileHeight)) {
    // Can fit only one tile
    DrawTexturePro(texture,
                   Rectangle{source.x, source.y,
                             ((float)dest.width / tileWidth) * source.width,
                             ((float)dest.height / tileHeight) * source.height},
                   Rectangle{dest.x, dest.y, dest.width, dest.height}, origin,
                   rotation, tint);
  } else if (dest.width <= tileWidth) {
    // Tiled vertically (one column)
    int dy = 0;
    for (; dy + tileHeight < dest.height; dy += tileHeight) {
      DrawTexturePro(
          texture,
          Rectangle{source.x, source.y,
                    ((float)dest.width / tileWidth) * source.width,
                    source.height},
          Rectangle{dest.x, dest.y + dy, dest.width, (float)tileHeight}, origin,
          rotation, tint);
    }

    // Fit last tile
    if (dy < dest.height) {
      DrawTexturePro(
          texture,
          Rectangle{source.x, source.y,
                    ((float)dest.width / tileWidth) * source.width,
                    ((float)(dest.height - dy) / tileHeight) * source.height},
          Rectangle{dest.x, dest.y + dy, dest.width, dest.height - dy}, origin,
          rotation, tint);
    }
  } else if (dest.height <= tileHeight) {
    // Tiled horizontally (one row)
    int dx = 0;
    for (; dx + tileWidth < dest.width; dx += tileWidth) {
      DrawTexturePro(
          texture,
          Rectangle{source.x, source.y, source.width,
                    ((float)dest.height / tileHeight) * source.height},
          Rectangle{dest.x + dx, dest.y, (float)tileWidth, dest.height}, origin,
          rotation, tint);
    }

    // Fit last tile
    if (dx < dest.width) {
      DrawTexturePro(
          texture,
          Rectangle{source.x, source.y,
                    ((float)(dest.width - dx) / tileWidth) * source.width,
                    ((float)dest.height / tileHeight) * source.height},
          Rectangle{dest.x + dx, dest.y, dest.width - dx, dest.height}, origin,
          rotation, tint);
    }
  } else {
    // Tiled both horizontally and vertically (rows and columns)
    int dx = 0;
    for (; dx + tileWidth < dest.width; dx += tileWidth) {
      int dy = 0;
      for (; dy + tileHeight < dest.height; dy += tileHeight) {
        DrawTexturePro(texture, source,
                       Rectangle{dest.x + dx, dest.y + dy, (float)tileWidth,
                                 (float)tileHeight},
                       origin, rotation, tint);
      }

      if (dy < dest.height) {
        DrawTexturePro(
            texture,
            Rectangle{source.x, source.y, source.width,
                      ((float)(dest.height - dy) / tileHeight) * source.height},
            Rectangle{dest.x + dx, dest.y + dy, (float)tileWidth,
                      dest.height - dy},
            origin, rotation, tint);
      }
    }

    // Fit last column of tiles
    if (dx < dest.width) {
      int dy = 0;
      for (; dy + tileHeight < dest.height; dy += tileHeight) {
        DrawTexturePro(
            texture,
            Rectangle{source.x, source.y,
                      ((float)(dest.width - dx) / tileWidth) * source.width,
                      source.height},
            Rectangle{dest.x + dx, dest.y + dy, dest.width - dx,
                      (float)tileHeight},
            origin, rotation, tint);
      }

      // Draw final tile in the bottom right corner
      if (dy < dest.height) {
        DrawTexturePro(
            texture,
            Rectangle{source.x, source.y,
                      ((float)(dest.width - dx) / tileWidth) * source.width,
                      ((float)(dest.height - dy) / tileHeight) * source.height},
            Rectangle{dest.x + dx, dest.y + dy, dest.width - dx,
                      dest.height - dy},
            origin, rotation, tint);
      }
    }
  }
}

}  // namespace raylib

namespace lime {

static const int kPauseIndexTable[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
};

static const int kCursorAlphaTable[] = {
    255, 247, 239, 231, 223, 215, 207, 199, 191, 183, 175, 167, 159, 151,
    143, 135, 127, 119, 111, 103, 95,  103, 111, 119, 127, 135, 143, 151,
    159, 167, 175, 183, 191, 199, 207, 215, 223, 231, 239, 247,
};

Window::Window(int x, int y, int width, int height)
    : rgss3_style_(Config::Instance()->rgss_version >= 3),
      ViewportChild(nullptr,
                    rgss3_style_ ? ZValue(100, std::numeric_limits<int>::max())
                                 : ZValue(0, 0)),
      window_skin_(nullptr),
      contents_(MakeRefCounted<Bitmap>(1, 1)),
      cursor_rect_(MakeRefCounted<Rect>()),
      x_(x),
      y_(y),
      width_(width),
      height_(height),
      padding_(rgss3_style_ ? 12 : 16),
      padding_bottom_(padding_),
      back_opacity_(rgss3_style_ ? 192 : 255),
      tone_(MakeRefCounted<Tone>()) {}

Window::Window() : Window(0, 0, 0, 0) {}

Window::~Window() {
  Dispose();
}

void Window::Update() {
  if (pause_) {
    pause_index_ += 1;
    pause_index_ = pause_index_ % std::size(kPauseIndexTable);
  }

  if (active_) {
    cursor_index_ += 1;
    cursor_index_ = cursor_index_ % std::size(kCursorAlphaTable);
  }
}

void Window::Move(int x, int y, int width, int height) {
  x_ = x;
  y_ = y;
  width_ = width;
  height_ = height;
}

bool Window::Opened() {
  return openness_ == 255;
}

bool Window::Closed() {
  return openness_ == 0;
}

ATTR_DEF(RefPtr<Bitmap>, WindowSkin, Window) {
  if (value.has_value()) {
    window_skin_ = *value;
    return std::nullopt;
  } else {
    return window_skin_;
  }
}

ATTR_DEF(RefPtr<Bitmap>, Contents, Window) {
  if (value.has_value()) {
    contents_ = *value;
    return std::nullopt;
  } else {
    return contents_;
  }
}

ATTR_DEF(RefPtr<Rect>, CursorRect, Window) {
  if (value.has_value()) {
    cursor_rect_->Set(*value);
    return std::nullopt;
  } else {
    return cursor_rect_;
  }
}

ATTR_DEF(bool, Active, Window) {
  if (value.has_value()) {
    active_ = *value;
    return std::nullopt;
  } else {
    return active_;
  }
}

ATTR_DEF(bool, ArrowsVisible, Window) {
  if (value.has_value()) {
    arrows_visible_ = *value;
    return std::nullopt;
  } else {
    return arrows_visible_;
  }
}

ATTR_DEF(bool, Pause, Window) {
  if (value.has_value()) {
    pause_ = *value;
    return std::nullopt;
  } else {
    return pause_;
  }
}

ATTR_DEF(int, X, Window) {
  if (value.has_value()) {
    x_ = *value;
    return std::nullopt;
  } else {
    return x_;
  }
}

ATTR_DEF(int, Y, Window) {
  if (value.has_value()) {
    y_ = *value;
    return std::nullopt;
  } else {
    return y_;
  }
}

ATTR_DEF(int, Width, Window) {
  if (value.has_value()) {
    width_ = *value;
    return std::nullopt;
  } else {
    return width_;
  }
}

ATTR_DEF(int, Height, Window) {
  if (value.has_value()) {
    height_ = *value;
    return std::nullopt;
  } else {
    return height_;
  }
}

ATTR_DEF(int, OX, Window) {
  if (value.has_value()) {
    ox_ = *value;
    return std::nullopt;
  } else {
    return ox_;
  }
}

ATTR_DEF(int, OY, Window) {
  if (value.has_value()) {
    oy_ = *value;
    return std::nullopt;
  } else {
    return oy_;
  }
}

ATTR_DEF(int, Padding, Window) {
  if (value.has_value()) {
    padding_ = *value;
    padding_bottom_ = *value;
    return std::nullopt;
  } else {
    return padding_;
  }
}

ATTR_DEF(int, PaddingBottom, Window) {
  if (value.has_value()) {
    padding_bottom_ = *value;
    return std::nullopt;
  } else {
    return padding_bottom_;
  }
}

ATTR_DEF(int, Opacity, Window) {
  if (value.has_value()) {
    opacity_ = std::clamp<int>(*value, 0, 255);
    return std::nullopt;
  } else {
    return opacity_;
  }
}

ATTR_DEF(int, BackOpacity, Window) {
  if (value.has_value()) {
    back_opacity_ = std::clamp<int>(*value, 0, 255);
    return std::nullopt;
  } else {
    return back_opacity_;
  }
}

ATTR_DEF(int, ContentsOpacity, Window) {
  if (value.has_value()) {
    contents_opacity_ = std::clamp<int>(*value, 0, 255);
    return std::nullopt;
  } else {
    return contents_opacity_;
  }
}

ATTR_DEF(int, Openness, Window) {
  if (value.has_value()) {
    openness_ = std::clamp<int>(*value, 0, 255);
    return std::nullopt;
  } else {
    return openness_;
  }
}

ATTR_DEF(int, Scale, Window) {
  if (value.has_value()) {
    scale_ = *value;
    return std::nullopt;
  } else {
    return scale_;
  }
}

ATTR_DEF(RefPtr<Tone>, Tone, Window) {
  if (value.has_value()) {
    tone_->Set(*value);
    return std::nullopt;
  } else {
    return tone_;
  }
}

void Window::DisposeObject() {
  Drawable::RemoveFromList();
}

void Window::Draw(DrawParam param) {
  const float fx = static_cast<float>(x_);
  const float fy = static_cast<float>(y_);
  const float fw = static_cast<float>(width_);
  const float fh = static_cast<float>(height_);
  const float fscale = static_cast<float>(scale_);
  const float fpad = static_cast<float>(padding_);
  const float fpad_bottom = static_cast<float>(padding_bottom_);

  raylib::Rectangle padding_rect = {fpad, fpad,
                                    std::max(0.0f, fw - fpad * 2.0f),
                                    std::max(0.0f, fh - (fpad + fpad_bottom))};

  raylib::rlEnableColorBlend();
  raylib::rlSetBlendMode(raylib::BLEND_ALPHA_PREMULTIPLY);

  if (width_ >= scale_ * 2 && height_ >= scale_ * 2) {
    // Window frame & background
    if (auto windowskin = window_skin_; window_skin_) {
      auto& skin_texture = window_skin_->render_texture().texture;

      // Source
      const raylib::Rectangle background1_src =
          raylib::IntRect(0.0f, 0.0f, 32.0f * fscale, 32.0f * fscale);
      const raylib::Rectangle background2_src =
          raylib::IntRect(0.0f, 32.0f * fscale, 32.0f * fscale, 32.0f * fscale);

      const raylib::Rectangle corner_left_top_src =
          raylib::IntRect(32.0f * fscale, 0.0f, 8.0f * fscale, 8.0f * fscale);
      const raylib::Rectangle corner_right_top_src =
          raylib::IntRect(56.0f * fscale, 0.0f, 8.0f * fscale, 8.0f * fscale);
      const raylib::Rectangle corner_left_bottom_src = raylib::IntRect(
          32.0f * fscale, 24.0f * fscale, 8.0f * fscale, 8.0f * fscale);
      const raylib::Rectangle corner_right_bottom_src = raylib::IntRect(
          56.0f * fscale, 24.0f * fscale, 8.0f * fscale, 8.0f * fscale);

      const raylib::Rectangle frame_up_src =
          raylib::IntRect(40.0f * fscale, 0.0f, 16.0f * fscale, 8.0f * fscale);
      const raylib::Rectangle frame_down_src = raylib::IntRect(
          40.0f * fscale, 24.0f * fscale, 16.0f * fscale, 8.0f * fscale);
      const raylib::Rectangle frame_left_src = raylib::IntRect(
          32.0f * fscale, 8.0f * fscale, 8.0f * fscale, 16.0f * fscale);
      const raylib::Rectangle frame_right_src = raylib::IntRect(
          56.0f * fscale, 8.0f * fscale, 8.0f * fscale, 16.0f * fscale);

      // Destination
      const raylib::Rectangle background_dest = raylib::IntRect(
          fx + fscale, fy + fscale, fw - 2.0f * fscale, fh - 2.0f * fscale);

      const raylib::Rectangle corner_left_top_dest =
          raylib::IntRect(fx, fy, 8.0f * fscale, 8.0f * fscale);
      const raylib::Rectangle corner_right_top_dest = raylib::IntRect(
          fx + fw - 8.0f * fscale, fy, 8.0f * fscale, 8.0f * fscale);
      const raylib::Rectangle corner_left_bottom_dest = raylib::IntRect(
          fx, fy + fh - 8.0f * fscale, 8.0f * fscale, 8.0f * fscale);
      const raylib::Rectangle corner_right_bottom_dest =
          raylib::IntRect(fx + fw - 8.0f * fscale, fy + fh - 8.0f * fscale,
                          8.0f * fscale, 8.0f * fscale);

      const raylib::Rectangle frame_up_dest = {
          fx + 8.0f * fscale, fy, fw - 16.0f * fscale, 8.0f * fscale};
      const raylib::Rectangle frame_down_dest = {
          fx + 8.0f * fscale, fy + fh - 8.0f * fscale, fw - 16.0f * fscale,
          8.0f * fscale};
      const raylib::Rectangle frame_left_dest = {
          fx, fy + 8.0f * fscale, 8.0f * fscale, fh - 16.0f * fscale};
      const raylib::Rectangle frame_right_dest = {
          fx + fw - 8.0f * fscale, fy + 8.0f * fscale, 8.0f * fscale,
          fh - 16.0f * fscale};

      raylib::rlMatrixMode(RL_MODELVIEW);
      raylib::rlPushMatrix();
      {
        // Openness
        const float center_y = fy + fh / 2.0f;
        raylib::rlTranslatef(0.0f, center_y, 0.0f);
        raylib::rlScalef(1.0f, openness_ / 255.0f, 1.0f);
        raylib::rlTranslatef(0.0f, -center_y, 0.0f);

        auto& shader = ShaderSet::Instance()->viewport;
        raylib::BeginShaderMode(shader.shader);
        {
          const raylib::Vector4 color_norm = {};
          const raylib::Vector4 tone_norm = tone_->Normalize();
          const float opacity_norm =
              (opacity_ * back_opacity_) / (255.0f * 255.0f);

          raylib::SetShaderValue(shader.shader, shader.u_color, &color_norm,
                                 raylib::SHADER_UNIFORM_VEC4);
          raylib::SetShaderValue(shader.shader, shader.u_tone, &tone_norm,
                                 raylib::SHADER_UNIFORM_VEC4);
          raylib::SetShaderValue(shader.shader, shader.u_opacity, &opacity_norm,
                                 raylib::SHADER_UNIFORM_FLOAT);

          // 1. Stretch layer
          raylib::DrawTexturePro(skin_texture, background1_src, background_dest,
                                 {}, 0.0f, {});
        }
        raylib::EndShaderMode();

        // 2. Tiled layer
        raylib::DrawTextureTiled(
            skin_texture, background2_src, background_dest, {}, 0, 1,
            raylib::MakeColor((opacity_ * back_opacity_) / 255));

        // 3. Corners & frames
        const raylib::Color frame_tint = raylib::MakeColor(opacity_);
        raylib::DrawTexturePro(skin_texture, corner_left_top_src,
                               corner_left_top_dest, {}, 0.0f, frame_tint);
        raylib::DrawTexturePro(skin_texture, corner_right_top_src,
                               corner_right_top_dest, {}, 0.0f, frame_tint);
        raylib::DrawTexturePro(skin_texture, corner_left_bottom_src,
                               corner_left_bottom_dest, {}, 0.0f, frame_tint);
        raylib::DrawTexturePro(skin_texture, corner_right_bottom_src,
                               corner_right_bottom_dest, {}, 0.0f, frame_tint);

        raylib::DrawTextureTiled(skin_texture, frame_up_src, frame_up_dest, {},
                                 0.0f, 1.0f, frame_tint);
        raylib::DrawTextureTiled(skin_texture, frame_down_src, frame_down_dest,
                                 {}, 0.0f, 1.0f, frame_tint);
        raylib::DrawTextureTiled(skin_texture, frame_left_src, frame_left_dest,
                                 {}, 0.0f, 1.0f, frame_tint);
        raylib::DrawTextureTiled(skin_texture, frame_right_src,
                                 frame_right_dest, {}, 0.0f, 1.0f, frame_tint);
      }
      raylib::rlPopMatrix();

      if (openness_ == 255) {
        // 4. Arrows
        const raylib::Vector2 arrow_display_offset = {
            fx + (fw - 8.0f * fscale) / 2.0f, fy + (fh - 8.0f * fscale) / 2.0f};

        if (arrows_visible_) {
          const raylib::Rectangle arrow_up_dest =
              raylib::IntRect(arrow_display_offset.x, fy + 2.0f * fscale,
                              8.0f * fscale, 4.0f * fscale);
          const raylib::Rectangle arrow_down_dest =
              raylib::IntRect(arrow_display_offset.x, fy + fh - 6.0f * fscale,
                              8.0f * fscale, 4.0f * fscale);
          const raylib::Rectangle arrow_left_dest =
              raylib::IntRect(fx + 2.0f * fscale, arrow_display_offset.y,
                              4.0f * fscale, 8.0f * fscale);
          const raylib::Rectangle arrow_right_dest =
              raylib::IntRect(fx + fw - 6.0f * fscale, arrow_display_offset.y,
                              4.0f * fscale, 8.0f * fscale);

          const raylib::Rectangle arrow_up_src = raylib::IntRect(
              44.0f * fscale, 8.0f * fscale, 8.0f * fscale, 4.0f * fscale);
          const raylib::Rectangle arrow_down_src = raylib::IntRect(
              44.0f * fscale, 20.0f * fscale, 8.0f * fscale, 4.0f * fscale);
          const raylib::Rectangle arrow_left_src = raylib::IntRect(
              40.0f * fscale, 12.0f * fscale, 4.0f * fscale, 8.0f * fscale);
          const raylib::Rectangle arrow_right_src = raylib::IntRect(
              52.0f * fscale, 12.0f * fscale, 4.0f * fscale, 8.0f * fscale);

          if (contents_) {
            if (ox_ > 0)
              raylib::DrawTexturePro(skin_texture, arrow_left_src,
                                     arrow_left_dest, {}, 0.0f, raylib::WHITE);
            if (oy_ > 0)
              raylib::DrawTexturePro(skin_texture, arrow_up_src, arrow_up_dest,
                                     {}, 0.0f, raylib::WHITE);
            if (padding_rect.width <
                static_cast<float>(contents_->Width() - ox_))
              raylib::DrawTexturePro(skin_texture, arrow_right_src,
                                     arrow_right_dest, {}, 0.0f, raylib::WHITE);
            if (padding_rect.height <
                static_cast<float>(contents_->Height() - oy_))
              raylib::DrawTexturePro(skin_texture, arrow_down_src,
                                     arrow_down_dest, {}, 0.0f, raylib::WHITE);
          }
        }

        // 5. Pause
        if (pause_) {
          const raylib::Rectangle pause_src[] = {
              {48.0f * fscale, 32.0f * fscale, 8.0f * fscale, 8.0f * fscale},
              {56.0f * fscale, 32.0f * fscale, 8.0f * fscale, 8.0f * fscale},
              {48.0f * fscale, 40.0f * fscale, 8.0f * fscale, 8.0f * fscale},
              {56.0f * fscale, 40.0f * fscale, 8.0f * fscale, 8.0f * fscale},
          };

          const raylib::Rectangle pause_dest =
              raylib::IntRect(arrow_display_offset.x, fy + fh - 8.0f * fscale,
                              8.0f * fscale, 8.0f * fscale);

          raylib::DrawTexturePro(skin_texture,
                                 pause_src[kPauseIndexTable[pause_index_]],
                                 pause_dest, {}, 0.0f, raylib::WHITE);
        }
      }
    }

    if (openness_ == 255) {
      if (auto windowskin = window_skin_; window_skin_) {
        auto& skin_texture = window_skin_->render_texture().texture;

        // 6. Cursor
        const raylib::Color contents_tint = raylib::MakeColor(
            contents_opacity_ * kCursorAlphaTable[cursor_index_] / 255);
        const auto cursor_rect = cursor_rect_->As();
        if (cursor_rect.width > 0 && cursor_rect.height > 0) {
          auto build_cursor_internal = [&](const raylib::Rectangle& rect,
                                           raylib::Rectangle quad_rects[9],
                                           int32_t unit) {
            const int32_t w = rect.width;
            const int32_t h = rect.height;
            const int32_t l = rect.x;
            const int32_t r = l + w;
            const int32_t t = rect.y;
            const int32_t b = t + h;

            int32_t i = 0;
            // Left-Top
            quad_rects[i++] = raylib::IntRect(l, t, unit, unit);
            // Right-Top
            quad_rects[i++] = raylib::IntRect(r - unit, t, unit, unit);
            // Right-Bottom
            quad_rects[i++] = raylib::IntRect(r - unit, b - unit, unit, unit);
            // Left-Bottom
            quad_rects[i++] = raylib::IntRect(l, b - unit, unit, unit);

            // Left
            quad_rects[i++] = raylib::IntRect(l, t + unit, unit, h - unit * 2);
            // Right
            quad_rects[i++] =
                raylib::IntRect(r - unit, t + unit, unit, h - unit * 2);
            // Top
            quad_rects[i++] = raylib::IntRect(l + unit, t, w - unit * 2, unit);
            // Bottom
            quad_rects[i++] =
                raylib::IntRect(l + unit, b - unit, w - unit * 2, unit);
            // Center
            quad_rects[i++] =
                raylib::IntRect(l + unit, t + unit, w - unit * 2, h - unit * 2);
          };

          // Manual glScissor: clip the 9-slice cursor to the window content
          // area (#padding_rect, in window coordinates). Each quad's dest is
          // intersected with the clip region, then the intersection is mapped
          // back to the source. In a 9-slice each source rect maps to its dest
          // by an independent per-axis scale (the four corners are 1:1, the
          // borders and the center stretch), so the inverse mapping is linear.
          const raylib::Rectangle clip_rect =
              raylib::IntRect(x_ + padding_rect.x, y_ + padding_rect.y,
                              padding_rect.width, padding_rect.height);

          auto build_cursor_quads = [&](const raylib::Rectangle& src,
                                        const raylib::Rectangle& dst) {
            const int32_t cursor_scale = scale_ >= 4 ? scale_ * 2 : 4;

            raylib::Rectangle texcoords[9], positions[9];
            build_cursor_internal(src, texcoords, cursor_scale);
            build_cursor_internal(dst, positions, cursor_scale);

            for (int32_t i = 0; i < 9; ++i) {
              const raylib::Rectangle& dst_rect = positions[i];
              const raylib::Rectangle& src_rect = texcoords[i];

              // Region intersection of this quad's dest with the clip area.
              const float clip_left = std::max(dst_rect.x, clip_rect.x);
              const float clip_top = std::max(dst_rect.y, clip_rect.y);
              const float clip_right = std::min(dst_rect.x + dst_rect.width,
                                                clip_rect.x + clip_rect.width);
              const float clip_bottom = std::min(
                  dst_rect.y + dst_rect.height, clip_rect.y + clip_rect.height);
              const float clip_width = clip_right - clip_left;
              const float clip_height = clip_bottom - clip_top;
              if (clip_width <= 0.0f || clip_height <= 0.0f)
                continue;

              // Inverse 9-slice mapping:
              //   src - src_rect = (dst - dst_rect) * (src_rect.size /
              //   dst_rect.size)
              const float sx_scale = dst_rect.width != 0.0f
                                         ? src_rect.width / dst_rect.width
                                         : 0.0f;
              const float sy_scale = dst_rect.height != 0.0f
                                         ? src_rect.height / dst_rect.height
                                         : 0.0f;

              const raylib::Rectangle clipped_src = {
                  src_rect.x + (clip_left - dst_rect.x) * sx_scale,
                  src_rect.y + (clip_top - dst_rect.y) * sy_scale,
                  clip_width * sx_scale, clip_height * sy_scale};

              raylib::DrawTexturePro(
                  skin_texture, clipped_src,
                  {clip_left, clip_top, clip_width, clip_height}, {}, 0,
                  contents_tint);
            }
          };

          const raylib::Rectangle cursor_src = raylib::IntRect(
              32 * scale_, 32 * scale_, 16 * scale_, 16 * scale_);
          if (cursor_rect.width > 0 && cursor_rect.height > 0) {
            raylib::Rectangle cursor_dest =
                raylib::IntRect(x_ + padding_rect.x + cursor_rect.x,
                                y_ + padding_rect.y + cursor_rect.y,
                                cursor_rect.width, cursor_rect.height);

            if (rgss3_style_) {
              cursor_dest.x -= ox_;
              cursor_dest.y -= oy_;
            }

            build_cursor_quads(cursor_src, cursor_dest);
          }
        }
      }

      // 7. Contents
      if (contents_) {
        auto& contents_texture = contents_->render_texture().texture;

        raylib::Rectangle s = {}, d = {};
        s.x = 0;
        s.y = 0;
        s.width = contents_texture.width;
        s.height = contents_texture.height;

        d.x = x_ + padding_rect.x - ox_;
        d.y = y_ + padding_rect.y - oy_;
        d.width = contents_texture.width;
        d.height = contents_texture.height;

        // Manual glScissor: clip the contents draw to the window content
        // area (#padding_rect, in window coordinates). This draw is 1:1
        // (no stretch: source size == dest size == texture size), so the
        // inverse mapping from dest to source is a plain offset:
        //   src = s + (clip - d)
        const raylib::Rectangle clip_rect =
            raylib::IntRect(x_ + padding_rect.x, y_ + padding_rect.y,
                            padding_rect.width, padding_rect.height);

        const float clip_left = std::max(d.x, clip_rect.x);
        const float clip_top = std::max(d.y, clip_rect.y);
        const float clip_right =
            std::min(d.x + d.width, clip_rect.x + clip_rect.width);
        const float clip_bottom =
            std::min(d.y + d.height, clip_rect.y + clip_rect.height);
        const float clip_width = clip_right - clip_left;
        const float clip_height = clip_bottom - clip_top;
        if (clip_width > 0.0f && clip_height > 0.0f) {
          const raylib::Rectangle clipped_src = {s.x + (clip_left - d.x),
                                                 s.y + (clip_top - d.y),
                                                 clip_width, clip_height};
          raylib::DrawTexturePro(contents_texture, clipped_src,
                                 {clip_left, clip_top, clip_width, clip_height},
                                 {}, 0, raylib::MakeColor(contents_opacity_));
        }
      }
    }
  }
}

}  // namespace lime
