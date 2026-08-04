#include "src/window.h"

#include "src/shader.h"

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

namespace rgssx {

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
    : ViewportChild(nullptr, ZValue(100, std::numeric_limits<int>::max())),
      window_skin_(nullptr),
      contents_(MakeRefCounted<Bitmap>(1, 1)),
      cursor_rect_(MakeRefCounted<Rect>()),
      x_(x),
      y_(y),
      width_(width),
      height_(height),
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

  raylib::Rectangle padding_rect{fpad, fpad, std::max(0.0f, fw - fpad * 2.0f),
                                 std::max(0.0f, fh - (fpad + fpad_bottom))};

  raylib::rlEnableColorBlend();
  raylib::rlSetBlendMode(raylib::BLEND_ALPHA_PREMULTIPLY);

  if (width_ >= scale_ * 2 && height_ >= scale_ * 2) {
    RectRegion window_rect = {static_cast<int>(x_ + padding_rect.x),
                              static_cast<int>(y_ + padding_rect.y),
                              static_cast<int>(padding_rect.width),
                              static_cast<int>(padding_rect.height)};
    auto window_scissor = RectRegion::MakeIntersect(param.scissor, window_rect);

    // Window frame & background
    if (auto windowskin = window_skin_; window_skin_) {
      auto& skin_texture = window_skin_->render_texture().texture;

      // Source
      const raylib::Rectangle background1_src(0.0f, 0.0f, 32.0f * fscale,
                                              32.0f * fscale);
      const raylib::Rectangle background2_src(0.0f, 32.0f * fscale,
                                              32.0f * fscale, 32.0f * fscale);

      const raylib::Rectangle corner_left_top_src(32.0f * fscale, 0.0f,
                                                  8.0f * fscale, 8.0f * fscale);
      const raylib::Rectangle corner_right_top_src(
          56.0f * fscale, 0.0f, 8.0f * fscale, 8.0f * fscale);
      const raylib::Rectangle corner_left_bottom_src(
          32.0f * fscale, 24.0f * fscale, 8.0f * fscale, 8.0f * fscale);
      const raylib::Rectangle corner_right_bottom_src(
          56.0f * fscale, 24.0f * fscale, 8.0f * fscale, 8.0f * fscale);

      const raylib::Rectangle frame_up_src(40.0f * fscale, 0.0f, 16.0f * fscale,
                                           8.0f * fscale);
      const raylib::Rectangle frame_down_src(40.0f * fscale, 24.0f * fscale,
                                             16.0f * fscale, 8.0f * fscale);
      const raylib::Rectangle frame_left_src(32.0f * fscale, 8.0f * fscale,
                                             8.0f * fscale, 16.0f * fscale);
      const raylib::Rectangle frame_right_src(56.0f * fscale, 8.0f * fscale,
                                              8.0f * fscale, 16.0f * fscale);

      // Destination
      const raylib::Rectangle background_dest(
          fx + fscale, fy + fscale, fw - 2.0f * fscale, fh - 2.0f * fscale);

      const raylib::Rectangle corner_left_top_dest(fx, fy, 8.0f * fscale,
                                                   8.0f * fscale);
      const raylib::Rectangle corner_right_top_dest(
          fx + fw - 8.0f * fscale, fy, 8.0f * fscale, 8.0f * fscale);
      const raylib::Rectangle corner_left_bottom_dest(
          fx, fy + fh - 8.0f * fscale, 8.0f * fscale, 8.0f * fscale);
      const raylib::Rectangle corner_right_bottom_dest(
          fx + fw - 8.0f * fscale, fy + fh - 8.0f * fscale, 8.0f * fscale,
          8.0f * fscale);

      const raylib::Rectangle frame_up_dest{fx + 8.0f * fscale, fy,
                                            fw - 16.0f * fscale, 8.0f * fscale};
      const raylib::Rectangle frame_down_dest{
          fx + 8.0f * fscale, fy + fh - 8.0f * fscale, fw - 16.0f * fscale,
          8.0f * fscale};
      const raylib::Rectangle frame_left_dest{
          fx, fy + 8.0f * fscale, 8.0f * fscale, fh - 16.0f * fscale};
      const raylib::Rectangle frame_right_dest{
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
          const raylib::Rectangle arrow_up_dest(arrow_display_offset.x,
                                                fy + 2.0f * fscale,
                                                8.0f * fscale, 4.0f * fscale);
          const raylib::Rectangle arrow_down_dest(arrow_display_offset.x,
                                                  fy + fh - 6.0f * fscale,
                                                  8.0f * fscale, 4.0f * fscale);
          const raylib::Rectangle arrow_left_dest(fx + 2.0f * fscale,
                                                  arrow_display_offset.y,
                                                  4.0f * fscale, 8.0f * fscale);
          const raylib::Rectangle arrow_right_dest(
              fx + fw - 6.0f * fscale, arrow_display_offset.y, 4.0f * fscale,
              8.0f * fscale);

          const raylib::Rectangle arrow_up_src(44.0f * fscale, 8.0f * fscale,
                                               8.0f * fscale, 4.0f * fscale);
          const raylib::Rectangle arrow_down_src(44.0f * fscale, 20.0f * fscale,
                                                 8.0f * fscale, 4.0f * fscale);
          const raylib::Rectangle arrow_left_src(40.0f * fscale, 12.0f * fscale,
                                                 4.0f * fscale, 8.0f * fscale);
          const raylib::Rectangle arrow_right_src(
              52.0f * fscale, 12.0f * fscale, 4.0f * fscale, 8.0f * fscale);

          if (contents_) {
            if (ox_ > 0)
              raylib::DrawTexturePro(skin_texture, arrow_left_src,
                                     arrow_left_dest, {}, 0.0f,
                                     raylib::RAYWHITE);
            if (oy_ > 0)
              raylib::DrawTexturePro(skin_texture, arrow_up_src, arrow_up_dest,
                                     {}, 0.0f, raylib::RAYWHITE);
            if (padding_rect.width <
                static_cast<float>(contents_->Width() - ox_))
              raylib::DrawTexturePro(skin_texture, arrow_right_src,
                                     arrow_right_dest, {}, 0.0f,
                                     raylib::RAYWHITE);
            if (padding_rect.height <
                static_cast<float>(contents_->Height() - oy_))
              raylib::DrawTexturePro(skin_texture, arrow_down_src,
                                     arrow_down_dest, {}, 0.0f,
                                     raylib::RAYWHITE);
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

          const raylib::Rectangle pause_dest(arrow_display_offset.x,
                                             fy + fh - 8.0f * fscale,
                                             8.0f * fscale, 8.0f * fscale);

          raylib::DrawTexturePro(skin_texture,
                                 pause_src[kPauseIndexTable[pause_index_]],
                                 pause_dest, {}, 0.0f, raylib::RAYWHITE);
        }

        // 6. Cursor
        const raylib::Color contents_tint = raylib::MakeColor(
            contents_opacity_ * kCursorAlphaTable[cursor_index_] / 255);
        if (cursor_rect_->width > 0 && cursor_rect_->height > 0) {
          const raylib::Rectangle clip = {
              static_cast<float>(window_scissor.x),
              static_cast<float>(window_scissor.y),
              static_cast<float>(window_scissor.width),
              static_cast<float>(window_scissor.height)};

          const float cscale = static_cast<float>(scale_ >= 4 ? scale_ * 2 : 4);
          const float cw = static_cast<float>(cursor_rect_->width);
          const float ch = static_cast<float>(cursor_rect_->height);
          const float cx =
              fx + padding_rect.x + static_cast<float>(cursor_rect_->x);
          const float cy =
              fy + padding_rect.y + static_cast<float>(cursor_rect_->y);

          // Cursor source: 16x16 region at (32, 32) in windowskin
          const float ssx = 32.0f * fscale;
          const float ssy = 32.0f * fscale;
          const float ssw = 16.0f * fscale;
          const float ssh = 16.0f * fscale;

          // 9-patch: 4 corners + 4 edges + 1 center
          const raylib::Rectangle src[9] = {
              {ssx, ssy, cscale, cscale},
              {ssx + ssw - cscale, ssy, cscale, cscale},
              {ssx + ssw - cscale, ssy + ssh - cscale, cscale, cscale},
              {ssx, ssy + ssh - cscale, cscale, cscale},
              {ssx, ssy + cscale, cscale, ssh - cscale * 2.0f},
              {ssx + ssw - cscale, ssy + cscale, cscale, ssh - cscale * 2.0f},
              {ssx + cscale, ssy, ssw - cscale * 2.0f, cscale},
              {ssx + cscale, ssy + ssh - cscale, ssw - cscale * 2.0f, cscale},
              {ssx + cscale, ssy + cscale, ssw - cscale * 2.0f,
               ssh - cscale * 2.0f},
          };

          const raylib::Rectangle dst[9] = {
              {cx, cy, cscale, cscale},
              {cx + cw - cscale, cy, cscale, cscale},
              {cx + cw - cscale, cy + ch - cscale, cscale, cscale},
              {cx, cy + ch - cscale, cscale, cscale},
              {cx, cy + cscale, cscale, ch - cscale * 2.0f},
              {cx + cw - cscale, cy + cscale, cscale, ch - cscale * 2.0f},
              {cx + cscale, cy, cw - cscale * 2.0f, cscale},
              {cx + cscale, cy + ch - cscale, cw - cscale * 2.0f, cscale},
              {cx + cscale, cy + cscale, cw - cscale * 2.0f,
               ch - cscale * 2.0f},
          };

          for (int i = 0; i < 9; ++i) {
            raylib::Rectangle s = src[i];
            raylib::Rectangle d = dst[i];

            // Clip dst against window_scissor, adjust src proportionally
            const float left = std::max(d.x, clip.x);
            const float top = std::max(d.y, clip.y);
            const float right = std::min(d.x + d.width, clip.x + clip.width);
            const float bottom = std::min(d.y + d.height, clip.y + clip.height);

            if (left < right && top < bottom) {
              const float sx_ratio = (left - d.x) / d.width;
              const float sy_ratio = (top - d.y) / d.height;
              const float sw_ratio = (right - left) / d.width;
              const float sh_ratio = (bottom - top) / d.height;

              s.x += sx_ratio * s.width;
              s.y += sy_ratio * s.height;
              s.width *= sw_ratio;
              s.height *= sh_ratio;

              d.x = left;
              d.y = top;
              d.width = right - left;
              d.height = bottom - top;

              raylib::DrawTexturePro(skin_texture, s, d, {}, 0.0f,
                                     contents_tint);
            }
          }
        }
      }
    }

    // 7. Contents
    if (contents_ && openness_ == 255) {
      const raylib::Rectangle clip = {
          static_cast<float>(window_scissor.x),
          static_cast<float>(window_scissor.y),
          static_cast<float>(window_scissor.width),
          static_cast<float>(window_scissor.height)};

      auto& contents_texture = contents_->render_texture().texture;
      const float cx = fx + padding_rect.x - static_cast<float>(ox_);
      const float cy = fy + padding_rect.y - static_cast<float>(oy_);

      raylib::Rectangle s = {0.0f, 0.0f,
                             static_cast<float>(contents_texture.width),
                             static_cast<float>(contents_texture.height)};
      raylib::Rectangle d = {cx, cy, static_cast<float>(contents_texture.width),
                             static_cast<float>(contents_texture.height)};

      // Clip dst against window_scissor, adjust src proportionally
      const float left = std::max(d.x, clip.x);
      const float top = std::max(d.y, clip.y);
      const float right = std::min(d.x + d.width, clip.x + clip.width);
      const float bottom = std::min(d.y + d.height, clip.y + clip.height);

      if (left < right && top < bottom) {
        const float sx_ratio = (left - d.x) / d.width;
        const float sy_ratio = (top - d.y) / d.height;
        const float sw_ratio = (right - left) / d.width;
        const float sh_ratio = (bottom - top) / d.height;

        s.x += sx_ratio * s.width;
        s.y += sy_ratio * s.height;
        s.width *= sw_ratio;
        s.height *= sh_ratio;

        d.x = left;
        d.y = top;
        d.width = right - left;
        d.height = bottom - top;

        raylib::DrawTexturePro(contents_texture, s, d, {}, 0,
                               raylib::MakeColor(contents_opacity_));
      }
    }
  }
}

}  // namespace rgssx
