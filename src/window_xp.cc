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

#include "src/window_xp.h"

namespace lime {

WindowXPAbove::WindowXPAbove(WindowXP* parent, RefPtr<Viewport> viewport)
    : ViewportChild(viewport, ZValue(parent->Attr_Z().value() + 2)),
      parent_(parent) {}

void WindowXPAbove::Draw(DrawParam param) {
  parent_->DrawAbove(param);
}

// ----------------------------------------------------------------------

WindowXP::WindowXP(RefPtr<Viewport> viewport)
    : ViewportChild(viewport, ZValue()),
      above_(std::make_unique<WindowXPAbove>(this, viewport)),
      cursor_rect_(MakeRefCounted<Rect>()) {}

WindowXP::~WindowXP() {
  Dispose();
}

void WindowXP::Update() {
  ++pause_index_;
  if (pause_index_ >= 32)
    pause_index_ = 0;

  if (active_) {
    cursor_opacity_ += cursor_fade_ ? -8 : 8;
    if (cursor_opacity_ > 255) {
      cursor_opacity_ = 255;
      cursor_fade_ = true;
    } else if (cursor_opacity_ < 128) {
      cursor_opacity_ = 128;
      cursor_fade_ = false;
    }
  } else {
    cursor_opacity_ = 128;
  }
}

ATTR_DEF(RefPtr<Viewport>, Viewport, WindowXP) {
  above_->Attr_Viewport(value);
  return ViewportChild::Attr_Viewport(value);
}

ATTR_DEF(bool, Visible, WindowXP) {
  above_->Attr_Visible(value);
  return Drawable::Attr_Visible(value);
}

ATTR_DEF(int, Z, WindowXP) {
  if (value.has_value())
    above_->Attr_Z(*value + 2);
  return Drawable::Attr_Z(value);
}

ATTR_DEF(RefPtr<Bitmap>, WindowSkin, WindowXP) {
  if (value.has_value()) {
    windowskin_ = *value;
    return std::nullopt;
  } else {
    return windowskin_;
  }
}

ATTR_DEF(RefPtr<Bitmap>, Contents, WindowXP) {
  if (value.has_value()) {
    contents_ = *value;
    return std::nullopt;
  } else {
    return contents_;
  }
}

ATTR_DEF(bool, Stretch, WindowXP) {
  if (value.has_value()) {
    stretch_ = *value;
    return std::nullopt;
  } else {
    return stretch_;
  }
}

ATTR_DEF(RefPtr<Rect>, CursorRect, WindowXP) {
  if (value.has_value()) {
    cursor_rect_->Set(*value);
    return std::nullopt;
  } else {
    return cursor_rect_;
  }
}

ATTR_DEF(bool, Active, WindowXP) {
  if (value.has_value()) {
    active_ = *value;
    return std::nullopt;
  } else {
    return active_;
  }
}

ATTR_DEF(bool, Pause, WindowXP) {
  if (value.has_value()) {
    pause_ = *value;
    return std::nullopt;
  } else {
    return pause_;
  }
}

ATTR_DEF(int, X, WindowXP) {
  if (value.has_value()) {
    x_ = *value;
    return std::nullopt;
  } else {
    return x_;
  }
}

ATTR_DEF(int, Y, WindowXP) {
  if (value.has_value()) {
    y_ = *value;
    return std::nullopt;
  } else {
    return y_;
  }
}

ATTR_DEF(int, Width, WindowXP) {
  if (value.has_value()) {
    width_ = *value;
    return std::nullopt;
  } else {
    return width_;
  }
}

ATTR_DEF(int, Height, WindowXP) {
  if (value.has_value()) {
    height_ = *value;
    return std::nullopt;
  } else {
    return height_;
  }
}

ATTR_DEF(int, OX, WindowXP) {
  if (value.has_value()) {
    ox_ = *value;
    return std::nullopt;
  } else {
    return ox_;
  }
}

ATTR_DEF(int, OY, WindowXP) {
  if (value.has_value()) {
    oy_ = *value;
    return std::nullopt;
  } else {
    return oy_;
  }
}

ATTR_DEF(int, Opacity, WindowXP) {
  if (value.has_value()) {
    opacity_ = *value;
    return std::nullopt;
  } else {
    return opacity_;
  }
}

ATTR_DEF(int, BackOpacity, WindowXP) {
  if (value.has_value()) {
    back_opacity_ = *value;
    return std::nullopt;
  } else {
    return back_opacity_;
  }
}

ATTR_DEF(int, ContentsOpacity, WindowXP) {
  if (value.has_value()) {
    contents_opacity_ = *value;
    return std::nullopt;
  } else {
    return contents_opacity_;
  }
}

void WindowXP::DisposeObject() {
  Drawable::RemoveFromList();

  above_.reset();
  windowskin_.reset();
  contents_.reset();
}

void WindowXP::Draw(DrawParam param) {
  DrawGround(param);
}

void WindowXP::DrawGround(DrawParam param) {
  if (windowskin_ && !windowskin_->IsDisposed()) {
    auto& window_texture = windowskin_->handle().texture;

    raylib::rlEnableColorBlend();
    raylib::rlSetBlendMode(raylib::RL_BLEND_ALPHA_PREMULTIPLY);

    const auto dst_rect = raylib::IntRect(
        x_ + scale_, y_ + scale_, width_ - 2 * scale_, height_ - 2 * scale_);
    const auto bg_src = raylib::IntRect(0, 0, 64 * scale_, 64 * scale_);
    const auto bg_tint = raylib::MakeColor((opacity_ * back_opacity_) / 255);

    // Background
    if (stretch_) {
      raylib::DrawTexturePro(window_texture, bg_src, dst_rect, {}, 0, bg_tint);
    } else {
      raylib::DrawTextureTiled(window_texture, bg_src, dst_rect, {}, 0, 1,
                               bg_tint);
    }

    // Corners
    const auto corner_left_top =
        raylib::IntRect(64 * scale_, 0, 8 * scale_, 8 * scale_);
    const auto corner_right_top =
        raylib::IntRect(88 * scale_, 0, 8 * scale_, 8 * scale_);
    const auto corner_right_bottom =
        raylib::IntRect(88 * scale_, 24 * scale_, 8 * scale_, 8 * scale_);
    const auto corner_left_bottom =
        raylib::IntRect(64 * scale_, 24 * scale_, 8 * scale_, 8 * scale_);

    const auto frame_tint = raylib::MakeColor(opacity_);

    raylib::DrawTextureRec(window_texture, corner_left_top,
                           {static_cast<float>(x_), static_cast<float>(y_)},
                           frame_tint);
    raylib::DrawTextureRec(
        window_texture, corner_right_top,
        {static_cast<float>(x_ + width_ - 8 * scale_), static_cast<float>(y_)},
        frame_tint);
    raylib::DrawTextureRec(window_texture, corner_right_bottom,
                           {static_cast<float>(x_ + width_ - 8 * scale_),
                            static_cast<float>(y_ + height_ - 8 * scale_)},
                           frame_tint);
    raylib::DrawTextureRec(
        window_texture, corner_left_bottom,
        {static_cast<float>(x_), static_cast<float>(y_ + height_ - 8 * scale_)},
        frame_tint);

    // Frames
    const auto frame_up =
        raylib::IntRect(72 * scale_, 0, 16 * scale_, 8 * scale_);
    const auto frame_down =
        raylib::IntRect(72 * scale_, 24 * scale_, 16 * scale_, 8 * scale_);
    const auto frame_left =
        raylib::IntRect(64 * scale_, 8 * scale_, 8 * scale_, 16 * scale_);
    const auto frame_right =
        raylib::IntRect(88 * scale_, 8 * scale_, 8 * scale_, 16 * scale_);

    const auto frame_up_pos =
        raylib::IntRect(x_ + 8 * scale_, y_, width_ - 16 * scale_, 8 * scale_);
    const auto frame_down_pos =
        raylib::IntRect(x_ + 8 * scale_, y_ + height_ - 8 * scale_,
                        width_ - 16 * scale_, 8 * scale_);
    const auto frame_left_pos =
        raylib::IntRect(x_, y_ + 8 * scale_, 8 * scale_, height_ - 16 * scale_);
    const auto frame_right_pos =
        raylib::IntRect(x_ + width_ - 8 * scale_, y_ + 8 * scale_, 8 * scale_,
                        height_ - 16 * scale_);

    raylib::DrawTextureTiled(window_texture, frame_up, frame_up_pos, {}, 0, 1,
                             frame_tint);
    raylib::DrawTextureTiled(window_texture, frame_down, frame_down_pos, {}, 0,
                             1, frame_tint);
    raylib::DrawTextureTiled(window_texture, frame_left, frame_left_pos, {}, 0,
                             1, frame_tint);
    raylib::DrawTextureTiled(window_texture, frame_right, frame_right_pos, {},
                             0, 1, frame_tint);
  }
}

void WindowXP::DrawAbove(DrawParam param) {
  const auto cursor_rect = cursor_rect_->As();
  const bool has_cursor = (windowskin_ != nullptr) && cursor_rect.width > 0 &&
                          cursor_rect.height > 0;
  const bool has_contents = (contents_ != nullptr);

  // Stencil clip: restrict the cursor and the contents to the window
  // content area (#padding_rect, in window coordinates). The content area
  // is rasterized into the stencil buffer once (color writes disabled),
  // then the cursor and the contents are drawn with the stencil test
  // enabled (only fragments where the stencil value equals the reference
  // pass), so no manual per-quad clipping is required.
  if (has_cursor || has_contents) {
    const raylib::Rectangle clip_rect =
        raylib::IntRect(x_ + 8 * scale_, y_ + 8 * scale_, width_ - 16 * scale_,
                        height_ - 16 * scale_);

    raylib::rlDrawRenderBatchActive();

    raylib::rlEnableStencilTest();
    raylib::rlSetStencilClearValue(0);
    raylib::rlClearStencilBuffer();

    // Draw the content area into the stencil buffer (stencil = 1)
    // without touching the color buffer.
    raylib::rlColorMask(false, false, false, false);
    raylib::rlStencilFunc(RL_ALWAYS, 1, 0xFF);
    raylib::rlStencilOp(RL_KEEP, RL_KEEP, RL_REPLACE);
    raylib::rlStencilMask(0xFF);
    raylib::DrawRectangle(static_cast<int>(clip_rect.x),
                          static_cast<int>(clip_rect.y),
                          static_cast<int>(clip_rect.width),
                          static_cast<int>(clip_rect.height), raylib::WHITE);

    // Flush the stencil shape now, while color writes are disabled and
    // the stencil state is still ALWAYS/REPLACE. The render batch defers
    // the actual drawing, so any state change below would otherwise apply
    // to it.
    raylib::rlDrawRenderBatchActive();

    // Only pass where the stencil value is 1 and keep the stencil buffer
    // untouched during the content draws.
    raylib::rlColorMask(true, true, true, true);
    raylib::rlStencilFunc(RL_EQUAL, 1, 0xFF);
    raylib::rlStencilOp(RL_KEEP, RL_KEEP, RL_KEEP);
    raylib::rlStencilMask(0x00);
  }

  if (windowskin_ && !windowskin_->IsDisposed()) {
    auto& window_texture = windowskin_->handle().texture;

    raylib::rlEnableColorBlend();
    raylib::rlSetBlendMode(raylib::RL_BLEND_ALPHA_PREMULTIPLY);

    // Cursor render
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
      quad_rects[i++] = raylib::IntRect(l, t, unit, unit);         // Left-Top
      quad_rects[i++] = raylib::IntRect(r - unit, t, unit, unit);  // Right-Top
      quad_rects[i++] =
          raylib::IntRect(r - unit, b - unit, unit, unit);  // Right-Bottom
      quad_rects[i++] =
          raylib::IntRect(l, b - unit, unit, unit);  // Left-Bottom

      quad_rects[i++] =
          raylib::IntRect(l, t + unit, unit, h - unit * 2);  // Left
      quad_rects[i++] =
          raylib::IntRect(r - unit, t + unit, unit, h - unit * 2);  // Right
      quad_rects[i++] =
          raylib::IntRect(l + unit, t, w - unit * 2, unit);  // Top
      quad_rects[i++] =
          raylib::IntRect(l + unit, b - unit, w - unit * 2, unit);  // Bottom
      quad_rects[i++] = raylib::IntRect(l + unit, t + unit, w - unit * 2,
                                        h - unit * 2);  // Center
    };

    auto build_cursor_quads = [&](const raylib::Rectangle& src,
                                  const raylib::Rectangle& dst) {
      raylib::Rectangle texcoords[9], positions[9];

      build_cursor_internal(src, texcoords, scale_);
      build_cursor_internal(dst, positions, scale_);

      const auto cursor_tint =
          raylib::MakeColor((contents_opacity_ * cursor_opacity_) / 255);

      for (int32_t i = 0; i < 9; ++i)
        raylib::DrawTexturePro(window_texture, texcoords[i], positions[i], {},
                               0, cursor_tint);
    };

    // Cursor render (9)
    if (cursor_rect.width > 0 && cursor_rect.height > 0) {
      const auto cursor_dest_rect = raylib::IntRect(
          x_ + cursor_rect.x + 8 * scale_, y_ + cursor_rect.y + 8 * scale_,
          cursor_rect.width, cursor_rect.height);
      const auto cursor_src =
          raylib::IntRect(64 * scale_, 32 * scale_, 16 * scale_, 16 * scale_);

      build_cursor_quads(cursor_src, cursor_dest_rect);
    }

    // Arrows render (0-4)
    const raylib::Vector2 scroll = {(width_ - 8 * scale_) / 2.0f,
                                    (height_ - 8 * scale_) / 2.0f};

    const auto scroll_arrow_up_pos =
        raylib::IntRect(x_ + scroll.x, y_ + 2 * scale_, 8 * scale_, 4 * scale_);
    const auto scroll_arrow_down_pos = raylib::IntRect(
        x_ + scroll.x, y_ + height_ - 6 * scale_, 8 * scale_, 4 * scale_);
    const auto scroll_arrow_left_pos =
        raylib::IntRect(x_ + 2 * scale_, y_ + scroll.y, 4 * scale_, 8 * scale_);
    const auto scroll_arrow_right_pos = raylib::IntRect(
        x_ + width_ - 6 * scale_, y_ + scroll.y, 4 * scale_, 8 * scale_);

    if (contents_ && !contents_->IsDisposed()) {
      const auto scroll_arrow_up_src =
          raylib::IntRect(76 * scale_, 8 * scale_, 8 * scale_, 4 * scale_);
      const auto scroll_arrow_down_src =
          raylib::IntRect(76 * scale_, 20 * scale_, 8 * scale_, 4 * scale_);
      const auto scroll_arrow_left_src =
          raylib::IntRect(72 * scale_, 12 * scale_, 4 * scale_, 8 * scale_);
      const auto scroll_arrow_right_src =
          raylib::IntRect(84 * scale_, 12 * scale_, 4 * scale_, 8 * scale_);

      if (ox_ > 0)
        raylib::DrawTexturePro(window_texture, scroll_arrow_left_src,
                               scroll_arrow_left_pos, {}, 0, raylib::WHITE);
      if (oy_ > 0)
        raylib::DrawTexturePro(window_texture, scroll_arrow_up_src,
                               scroll_arrow_up_pos, {}, 0, raylib::WHITE);
      if ((width_ - 16 * scale_) < (contents_->GetWidth() - ox_))
        raylib::DrawTexturePro(window_texture, scroll_arrow_right_src,
                               scroll_arrow_right_pos, {}, 0, raylib::WHITE);
      if ((height_ - 16 * scale_) < (contents_->GetHeight() - oy_))
        raylib::DrawTexturePro(window_texture, scroll_arrow_down_src,
                               scroll_arrow_down_pos, {}, 0, raylib::WHITE);
    }

    // Pause render (0-1)
    raylib::Rectangle pause_animation[] = {
        raylib::IntRect(80 * scale_, 32 * scale_, 8 * scale_, 8 * scale_),
        raylib::IntRect(88 * scale_, 32 * scale_, 8 * scale_, 8 * scale_),
        raylib::IntRect(80 * scale_, 40 * scale_, 8 * scale_, 8 * scale_),
        raylib::IntRect(88 * scale_, 40 * scale_, 8 * scale_, 8 * scale_),
    };

    if (pause_) {
      const auto pause_pos =
          raylib::IntRect(x_ + (width_ - 8 * scale_) / 2,
                          y_ + height_ - 8 * scale_, 8 * scale_, 8 * scale_);

      raylib::DrawTexturePro(window_texture, pause_animation[pause_index_ / 8],
                             pause_pos, {}, 0, raylib::WHITE);
    }
  }

  if (contents_ && !contents_->IsDisposed()) {
    auto& window_texture = contents_->handle().texture;

    raylib::rlEnableColorBlend();
    raylib::rlSetBlendMode(raylib::RL_BLEND_ALPHA_PREMULTIPLY);

    raylib::DrawTexture(window_texture, x_ + 8 * scale_ - ox_,
                        y_ + 8 * scale_ - oy_,
                        raylib::MakeColor(contents_opacity_));
  }

  if (has_cursor || has_contents) {
    // Flush the remaining cursor/contents draws while the stencil test is
    // still active, before disabling it (the batch defers actual drawing).
    raylib::rlDrawRenderBatchActive();
    raylib::rlStencilMask(0xFF);
    raylib::rlDisableStencilTest();
  }
}

}  // namespace lime
