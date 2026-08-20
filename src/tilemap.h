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

#include "src/bitmap.h"
#include "src/common.h"
#include "src/refptr.h"
#include "src/table.h"
#include "src/utility.h"
#include "src/viewport.h"

namespace lime {

class Tilemap;

class TilemapAbove : public ViewportChild {
 public:
  TilemapAbove(Tilemap* parent, RefPtr<Viewport> viewport);

 private:
  void Draw(DrawParam param) override;

  Tilemap* parent_;
};

class Tilemap : public RefCounted<Tilemap>,
                public Dispoable,
                public ViewportChild {
 public:
  Tilemap(RefPtr<Viewport> viewport = nullptr);
  ~Tilemap();

  /*-export.begin-*/
  void Update();

  void SetBitmap(int index, RefPtr<Bitmap> bitmap);
  RefPtr<Bitmap> GetBitmap(int index);

  ATTR(RefPtr<Viewport>, Viewport) override;
  ATTR(bool, Visible) override;
  ATTR(int, Z) override;

  ATTR(RefPtr<Table>, MapData);
  ATTR(RefPtr<Table>, FlashData);
  ATTR(RefPtr<Table>, Flags);
  ATTR(int, OX);
  ATTR(int, OY);
  /*-export.end-*/

 private:
  friend class TilemapAbove;

  struct TileQuad {
    raylib::Rectangle source;
    raylib::Rectangle destination;
  };

  enum TileID {
    TILE_A1 = 0,
    TILE_A2,
    TILE_A3,
    TILE_A4,
    TILE_A5,
    TILE_B,
    TILE_C,
    TILE_D,
    TILE_E,
    TILE_NUMS,
  };

  void DisposeObject() override;
  void Draw(DrawParam param) override;

  void CreateShadowSet();
  void UpdateViewport(DrawParam param);
  void DrawMapData(bool above);

  bool rgss3_style_ = true;

  std::unique_ptr<TilemapAbove> above_;
  raylib::Texture shadow_texture_;
  raylib::Rectangle render_viewport_ = {};
  raylib::Vector2 render_offset_ = {};

  int flash_timer_ = 0;
  int flash_opacity_ = 0;
  int frame_index_ = 0;

  int regular_anim_ = 0;
  int waterfall_anim_ = 0;

  RefPtr<Bitmap> bitmaps_[TILE_NUMS] = {};
  RefPtr<Table> map_data_;
  RefPtr<Table> flash_data_;
  RefPtr<Table> flags_;
  int ox_ = 0, oy_ = 0;
  bool xrepeat_ = true, yrepeat_ = true;
  int tilesize_ = 32;
};

}  // namespace lime
