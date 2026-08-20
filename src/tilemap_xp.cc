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

#include "src/tilemap_xp.h"

#include "src/graphics.h"

namespace lime {

namespace {

const int kMaxPriorities = 5;

const raylib::Vector2 kAutotileSrcRegular[][4] = {
    {{1.0f, 2.0f}, {1.5f, 2.0f}, {1.0f, 2.5f}, {1.5f, 2.5f}},
    {{2.0f, 0.0f}, {1.5f, 2.0f}, {1.0f, 2.5f}, {1.5f, 2.5f}},
    {{1.0f, 2.0f}, {2.5f, 0.0f}, {1.0f, 2.5f}, {1.5f, 2.5f}},
    {{2.0f, 0.0f}, {2.5f, 0.0f}, {1.0f, 2.5f}, {1.5f, 2.5f}},
    {{1.0f, 2.0f}, {1.5f, 2.0f}, {1.0f, 2.5f}, {2.5f, 0.5f}},
    {{2.0f, 0.0f}, {1.5f, 2.0f}, {1.0f, 2.5f}, {2.5f, 0.5f}},
    {{1.0f, 2.0f}, {2.5f, 0.0f}, {1.0f, 2.5f}, {2.5f, 0.5f}},
    {{2.0f, 0.0f}, {2.5f, 0.0f}, {1.0f, 2.5f}, {2.5f, 0.5f}},
    {{1.0f, 2.0f}, {1.5f, 2.0f}, {2.0f, 0.5f}, {1.5f, 2.5f}},
    {{2.0f, 0.0f}, {1.5f, 2.0f}, {2.0f, 0.5f}, {1.5f, 2.5f}},
    {{1.0f, 2.0f}, {2.5f, 0.0f}, {2.0f, 0.5f}, {1.5f, 2.5f}},
    {{2.0f, 0.0f}, {2.5f, 0.0f}, {2.0f, 0.5f}, {1.5f, 2.5f}},
    {{1.0f, 2.0f}, {1.5f, 2.0f}, {2.0f, 0.5f}, {2.5f, 0.5f}},
    {{2.0f, 0.0f}, {1.5f, 2.0f}, {2.0f, 0.5f}, {2.5f, 0.5f}},
    {{1.0f, 2.0f}, {2.5f, 0.0f}, {2.0f, 0.5f}, {2.5f, 0.5f}},
    {{2.0f, 0.0f}, {2.5f, 0.0f}, {2.0f, 0.5f}, {2.5f, 0.5f}},
    {{0.0f, 2.0f}, {0.5f, 2.0f}, {0.0f, 2.5f}, {0.5f, 2.5f}},
    {{0.0f, 2.0f}, {2.5f, 0.0f}, {0.0f, 2.5f}, {0.5f, 2.5f}},
    {{0.0f, 2.0f}, {0.5f, 2.0f}, {0.0f, 2.5f}, {2.5f, 0.5f}},
    {{0.0f, 2.0f}, {2.5f, 0.0f}, {0.0f, 2.5f}, {2.5f, 0.5f}},
    {{1.0f, 1.0f}, {1.5f, 1.0f}, {1.0f, 1.5f}, {1.5f, 1.5f}},
    {{1.0f, 1.0f}, {1.5f, 1.0f}, {1.0f, 1.5f}, {2.5f, 0.5f}},
    {{1.0f, 1.0f}, {1.5f, 1.0f}, {2.0f, 0.5f}, {1.5f, 1.5f}},
    {{1.0f, 1.0f}, {1.5f, 1.0f}, {2.0f, 0.5f}, {2.5f, 0.5f}},
    {{2.0f, 2.0f}, {2.5f, 2.0f}, {2.0f, 2.5f}, {2.5f, 2.5f}},
    {{2.0f, 2.0f}, {2.5f, 2.0f}, {2.0f, 0.5f}, {2.5f, 2.5f}},
    {{2.0f, 0.0f}, {2.5f, 2.0f}, {2.0f, 2.5f}, {2.5f, 2.5f}},
    {{2.0f, 0.0f}, {2.5f, 2.0f}, {2.0f, 0.5f}, {2.5f, 2.5f}},
    {{1.0f, 3.0f}, {1.5f, 3.0f}, {1.0f, 3.5f}, {1.5f, 3.5f}},
    {{2.0f, 0.0f}, {1.5f, 3.0f}, {1.0f, 3.5f}, {1.5f, 3.5f}},
    {{1.0f, 3.0f}, {2.5f, 0.0f}, {1.0f, 3.5f}, {1.5f, 3.5f}},
    {{2.0f, 0.0f}, {2.5f, 0.0f}, {1.0f, 3.5f}, {1.5f, 3.5f}},
    {{0.0f, 2.0f}, {2.5f, 2.0f}, {0.0f, 2.5f}, {2.5f, 2.5f}},
    {{1.0f, 1.0f}, {1.5f, 1.0f}, {1.0f, 3.5f}, {1.5f, 3.5f}},
    {{0.0f, 1.0f}, {0.5f, 1.0f}, {0.0f, 1.5f}, {0.5f, 1.5f}},
    {{0.0f, 1.0f}, {0.5f, 1.0f}, {0.0f, 1.5f}, {2.5f, 0.5f}},
    {{2.0f, 1.0f}, {2.5f, 1.0f}, {2.0f, 1.5f}, {2.5f, 1.5f}},
    {{2.0f, 1.0f}, {2.5f, 1.0f}, {2.0f, 0.5f}, {2.5f, 1.5f}},
    {{2.0f, 3.0f}, {2.5f, 3.0f}, {2.0f, 3.5f}, {2.5f, 3.5f}},
    {{2.0f, 0.0f}, {2.5f, 3.0f}, {2.0f, 3.5f}, {2.5f, 3.5f}},
    {{0.0f, 3.0f}, {0.5f, 3.0f}, {0.0f, 3.5f}, {0.5f, 3.5f}},
    {{0.0f, 3.0f}, {2.5f, 0.0f}, {0.0f, 3.5f}, {0.5f, 3.5f}},
    {{0.0f, 1.0f}, {2.5f, 1.0f}, {0.0f, 1.5f}, {2.5f, 1.5f}},
    {{0.0f, 1.0f}, {0.5f, 1.0f}, {0.0f, 3.5f}, {0.5f, 3.5f}},
    {{0.0f, 3.0f}, {2.5f, 3.0f}, {0.0f, 3.5f}, {2.5f, 3.5f}},
    {{2.0f, 1.0f}, {2.5f, 1.0f}, {2.0f, 3.5f}, {2.5f, 3.5f}},
    {{0.0f, 1.0f}, {2.5f, 1.0f}, {0.0f, 3.5f}, {2.5f, 3.5f}},
    {{0.0f, 0.0f}, {0.5f, 0.0f}, {0.0f, 0.5f}, {0.5f, 0.5f}},
};

}  // namespace

TilemapXPAbove::TilemapXPAbove(TilemapXP* parent,
                               RefPtr<Viewport> viewport,
                               ZValue z,
                               int id)
    : ViewportChild(viewport, z), parent_(parent), id_(id) {}

void TilemapXPAbove::Draw(DrawParam param) {
  parent_->DrawLayer(id_);
}

// ----------------------------------------------------------------------

TilemapXP::TilemapXP(RefPtr<Viewport> viewport)
    : ViewportChild(viewport, ZValue()) {}

TilemapXP::~TilemapXP() {
  Dispose();
}

void TilemapXP::Update() {
  int max_anim_index = 1;
  for (auto& it : autotiles_)
    max_anim_index *= it.frames;
  anim_index_ = ++anim_index_ % (max_anim_index * 16);
}

void TilemapXP::SetTileset(RefPtr<Bitmap> bitmap) {
  tileset_ = bitmap;
}

RefPtr<Bitmap> TilemapXP::GetTileset() {
  return tileset_;
}

void TilemapXP::SetAutotile(int index, RefPtr<Bitmap> bitmap) {
  autotiles_[index].texture = bitmap;
  if (bitmap->GetHeight() > tilesize_) {
    autotiles_[index].frames = bitmap->GetWidth() / (tilesize_ * 3);
  } else {
    autotiles_[index].frames = bitmap->GetWidth() / tilesize_;
  }
}

RefPtr<Bitmap> TilemapXP::GetAutotile(int index) {
  return autotiles_[index].texture;
}

ATTR_DEF(RefPtr<Viewport>, Viewport, TilemapXP) {
  return ViewportChild::Attr_Viewport(value);
}

ATTR_DEF(bool, Visible, TilemapXP) {
  return Drawable::Attr_Visible(value);
}

ATTR_DEF(int, Z, TilemapXP) {
  return Drawable::Attr_Z(value);
}

ATTR_DEF(RefPtr<Table>, MapData, TilemapXP) {
  if (value.has_value()) {
    map_data_ = *value;
    return std::nullopt;
  } else {
    return map_data_;
  }
}

ATTR_DEF(RefPtr<Table>, FlashData, TilemapXP) {
  if (value.has_value()) {
    flash_data_ = *value;
    return std::nullopt;
  } else {
    return flash_data_;
  }
}

ATTR_DEF(RefPtr<Table>, Priorities, TilemapXP) {
  if (value.has_value()) {
    priorities_ = *value;
    return std::nullopt;
  } else {
    return priorities_;
  }
}

ATTR_DEF(int, OX, TilemapXP) {
  if (value.has_value()) {
    ox_ = *value;
    return std::nullopt;
  } else {
    return ox_;
  }
}

ATTR_DEF(int, OY, TilemapXP) {
  if (value.has_value()) {
    oy_ = *value;
    return std::nullopt;
  } else {
    return oy_;
  }
}

void TilemapXP::DisposeObject() {
  Drawable::RemoveFromList();

  aboves_.clear();
  tileset_.reset();
  for (auto& it : autotiles_)
    it.texture.reset();
}

void TilemapXP::Draw(DrawParam param) {
  UpdateViewport(param);
  UpdateAboves();
  UpdateOrder();
  ParseTiles();
  DrawLayer(0);
}

void TilemapXP::UpdateAboves() {
  auto viewport = Attr_Viewport();
  int viewport_width = 0, viewport_height = 0;
  if (viewport.has_value()) {
    viewport_width = viewport.value()->Attr_Rect().value()->width;
    viewport_height = viewport.value()->Attr_Rect().value()->height;
  } else {
    viewport_width = Graphics::Instance()->GetWidth();
    viewport_height = Graphics::Instance()->GetHeight();
  }

  const int above_layers_count = (viewport_height / tilesize_) +
                                 !!(viewport_height % tilesize_) + 2 +
                                 kMaxPriorities;
  if (above_layers_count != aboves_.size()) {
    aboves_.clear();
    aboves_.resize(above_layers_count);
    for (int i = 1; i <= above_layers_count; ++i) {
      // 1. Tiles with a priority of 0 always have a Z-coordinate of 0.
      // 2. Priority 1 tiles placed at the top edge of the screen have a
      // Z-coordinate of 64.
      // 3. Every time the priority increases by 1 or the next tile down is
      // selected, the Z-coordinate increases by 32.
      // 4. The Z-coordinate changes accordingly as the tilemap scrolls
      // vertically.
      auto above_node = std::make_unique<TilemapXPAbove>(
          this, Attr_Viewport().value(), ZValue(), i);
      aboves_.push_back(std::move(above_node));
    }
  }
}

void TilemapXP::UpdateOrder() {
  for (int i = 0; i < aboves_.size(); ++i) {
    // i -> 1 [2  3  4   5   6]  7
    // z -> 32 64 96 128 160 192 224
    const int layer_order = 32 * (render_viewport_.y + i) - oy_;
    aboves_[i]->Attr_Z(layer_order);
  }
}

void TilemapXP::UpdateViewport(DrawParam param) {
  auto viewport = Attr_Viewport();
  int viewport_ox = 0, viewport_oy = 0;
  int viewport_width = 0, viewport_height = 0;
  if (viewport.has_value()) {
    viewport_ox = viewport.value()->Attr_OX().value();
    viewport_oy = viewport.value()->Attr_OY().value();
    viewport_width = viewport.value()->Attr_Rect().value()->width;
    viewport_height = viewport.value()->Attr_Rect().value()->height;
  } else {
    viewport_ox = Graphics::Instance()->Attr_OX().value();
    viewport_oy = Graphics::Instance()->Attr_OY().value();
    viewport_width = Graphics::Instance()->GetWidth();
    viewport_height = Graphics::Instance()->GetHeight();
  }

  const int tilemap_real_ox = ox_ + viewport_ox,
            tilemap_real_oy = oy_ + viewport_oy;

  // Quad parsing viewport
  raylib::Rectangle new_viewport = {};
  new_viewport.x = tilemap_real_ox / tilesize_;
  new_viewport.y = tilemap_real_oy / tilesize_ - 1;
  new_viewport.width =
      (viewport_width / tilesize_) + !!(viewport_width % tilesize_) + 1;
  new_viewport.height =
      (viewport_height / tilesize_) + !!(viewport_height % tilesize_) + 2;
  render_viewport_ = new_viewport;

  // Rendering offset
  const int display_offset_x = tilemap_real_ox % tilesize_,
            display_offset_y = tilemap_real_oy % tilesize_;
  render_offset_ = raylib::Vector2(static_cast<float>(-display_offset_x),
                                   static_cast<float>(-display_offset_y));
  render_offset_.y -= tilesize_;

  // Apply viewport origin
  render_offset_.x += viewport_ox;
  render_offset_.y += viewport_oy;
}

void TilemapXP::ParseTiles() {
  auto set_autotile_pos = [&](raylib::Rectangle& pos, int32_t index) {
    switch (index) {
      case 0:  // Left Top
        break;
      case 1:  // Right Top
        pos.x += tilesize_ / 2.0f;
        break;
      case 2:  // Left Bottom
        pos.y += tilesize_ / 2.0f;
        break;
      case 3:  // Right Bottom
        pos.x += tilesize_ / 2.0f;
        pos.y += tilesize_ / 2.0f;
        break;
      default:
        break;
    }
  };

  auto get_priority = [&](int16_t tile_id) -> int32_t {
    if (!priorities_ || tile_id >= static_cast<int32_t>(priorities_->XSize()))
      return 0;

    int16_t value = priorities_->Get(tile_id, 0, 0);
    if (value > 5)
      return -1;

    return value;
  };

  auto process_autotile = [&](int x, int y, int16_t tile_id,
                              std::vector<TileQuad>* target) {
    // Autotile (0-7)
    int32_t autotile_id = tile_id / 48 - 1;
    // Pattern (0-47)
    int32_t pattern_id = tile_id % 48;

    // Autotile invalid check
    auto& autotile = autotiles_[autotile_id];
    if (autotile.texture && !autotile.texture->IsDisposed()) {
      // Generate from autotile type
      if (autotile.texture->GetHeight() >= tilesize_ * 4) {
        const raylib::Vector2* autotile_src_pos =
            kAutotileSrcRegular[pattern_id];
        for (int32_t i = 0; i < 4; ++i) {
          raylib::Rectangle tex_src = {};
          tex_src.x = (anim_index_ % autotile.frames) * tilesize_ * 3 +
                      autotile_src_pos[i].x * tilesize_ + 0.5f;
          tex_src.y = autotile_src_pos[i].y * tilesize_ + 0.5f;
          tex_src.width = 0.5f * tilesize_ - 1.0f;
          tex_src.height = 0.5f * tilesize_ - 1.0f;

          raylib::Rectangle chunk_pos = raylib::IntRect(
              x * tilesize_, y * tilesize_, tilesize_ / 2.0f, tilesize_ / 2.0f);
          set_autotile_pos(chunk_pos, i);

          TileQuad quad = {};
          quad.texture = autotile.texture->handle().texture;
          quad.destination = chunk_pos;
          quad.source = tex_src;
          target->push_back(quad);
        }
      } else if (autotile.texture->GetHeight() <= tilesize_) {
        const raylib::Rectangle single_tex =
            raylib::IntRect((anim_index_ % autotile.frames) * tilesize_ + 0.5f,
                            0.5f, tilesize_ - 1.0f, tilesize_ - 1.0f);
        const raylib::Rectangle single_pos =
            raylib::IntRect(x * tilesize_, y * tilesize_, tilesize_, tilesize_);

        TileQuad quad = {};
        quad.texture = autotile.texture->handle().texture;
        quad.destination = single_pos;
        quad.source = single_tex;
        target->push_back(quad);
      }
    }
  };

  auto value_wrap = [&](int32_t value, int32_t range) {
    int32_t res = value % range;
    return res < 0 ? res + range : res;
  };

  auto get_wrap_data = [&](RefPtr<Table> t, int32_t x, int32_t y,
                           int32_t z) -> int16_t {
    if (!t)
      return 0;

    auto tile_x = xrepeat_ ? value_wrap(x, t->XSize()) : x;
    auto tile_y = yrepeat_ ? value_wrap(y, t->YSize()) : y;

    if (!xrepeat_ && (x < 0 || x >= static_cast<int32_t>(t->XSize())))
      return 0;
    if (!yrepeat_ && (y < 0 || y >= static_cast<int32_t>(t->YSize())))
      return 0;

    return t->Get(tile_x, tile_y, z);
  };

  auto process_tile = [&](int32_t x, int32_t y, int32_t z) {
    int32_t tile_id = get_wrap_data(map_data_, x + render_viewport_.x,
                                    y + render_viewport_.y, z);

    if (tile_id < 48)
      return;

    int32_t priority = get_priority(tile_id);
    if (priority == -1)
      return;

    std::vector<TileQuad>* target;
    if (!priority) {
      // Ground layer
      target = &ground_cache_;
    } else {
      // Above multi layers
      target = &aboves_cache_[y + priority - 1];
    }

    if (tile_id < 48 * 8)
      return process_autotile(x, y, tile_id, target);

    int32_t tileset_id = tile_id - 48 * 8;
    int32_t tile_x = tileset_id % 8;
    int32_t tile_y = tileset_id / 8;

    raylib::Vector2 atlas_offset(static_cast<float>(tile_x),
                                 static_cast<float>(tile_y));
    raylib::Rectangle quad_tex = raylib::IntRect(
        atlas_offset.x * tilesize_ + 0.5f, atlas_offset.y * tilesize_ + 0.5f,
        tilesize_ - 1.0f, tilesize_ - 1.0f);
    raylib::Rectangle quad_pos =
        raylib::IntRect(x * tilesize_, y * tilesize_, tilesize_, tilesize_);

    TileQuad quad = {};
    quad.texture = tileset_->handle().texture;
    quad.destination = quad_pos;
    quad.source = quad_tex;
    target->push_back(quad);
  };

  auto process_buffer = [&]() {
    for (int32_t x = 0; x < render_viewport_.width; ++x)
      for (int32_t y = 0; y < render_viewport_.height; ++y)
        for (int32_t z = 0; z < static_cast<int32_t>(map_data_->ZSize()); ++z)
          process_tile(x, y, z);
  };

  ground_cache_.clear();
  aboves_cache_.clear();
  aboves_cache_.resize(aboves_.size());

  process_buffer();
}

void TilemapXP::DrawLayer(int id) {
  // Blend state
  raylib::rlEnableColorBlend();
  raylib::rlSetBlendMode(raylib::RL_BLEND_ALPHA_PREMULTIPLY);

  // Target choose
  std::vector<TileQuad>* target = nullptr;
  if (id == 0) {
    target = &ground_cache_;
  } else {
    target = &aboves_cache_[id - 1];
  }

  for (auto& quad : *target)
    raylib::DrawTexturePro(quad.texture, quad.source, quad.destination, {}, 0,
                           raylib::WHITE);
}

}  // namespace lime
