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

class TilemapXP;

class TilemapXPAbove : public ViewportChild {
 public:
  TilemapXPAbove(TilemapXP* parent,
                 RefPtr<Viewport> viewport,
                 ZValue z,
                 int id);

 private:
  void Draw(DrawParam param) override;

  TilemapXP* parent_ = nullptr;
  int id_ = 0;
};

class TilemapXP : public RefCounted<TilemapXP>,
                  public Dispoable,
                  public ViewportChild {
 public:
  TilemapXP(RefPtr<Viewport> viewport = nullptr);
  ~TilemapXP();

  /*-export.begin-*/
  void Update();

  void SetTileset(RefPtr<Bitmap> bitmap);
  RefPtr<Bitmap> GetTileset();
  void SetAutotile(int index, RefPtr<Bitmap> bitmap);
  RefPtr<Bitmap> GetAutotile(int index);

  ATTR(RefPtr<Viewport>, Viewport) override;
  ATTR(bool, Visible) override;
  ATTR(int, Z) override;

  ATTR(RefPtr<Table>, MapData);
  ATTR(RefPtr<Table>, FlashData);
  ATTR(RefPtr<Table>, Priorities);
  ATTR(int, OX);
  ATTR(int, OY);
  /*-export.end-*/

 private:
  friend class TilemapXPAbove;

  struct TileQuad {
    raylib::Texture2D texture;
    raylib::Rectangle source;
    raylib::Rectangle destination;
  };

  void DisposeObject() override;
  void Draw(DrawParam param) override;

  void UpdateViewport(DrawParam param);
  void UpdateAboves();
  void UpdateOrder();
  void ParseTiles();
  void DrawLayer(int id);

  std::vector<std::unique_ptr<TilemapXPAbove>> aboves_;
  raylib::Rectangle render_viewport_ = {};
  raylib::Vector2 render_offset_ = {};
  int32_t anim_index_ = 0;

  RefPtr<Bitmap> tileset_;
  struct {
    RefPtr<Bitmap> texture;
    int frames = 0;
  } autotiles_[7] = {};
  RefPtr<Table> map_data_;
  RefPtr<Table> flash_data_;
  RefPtr<Table> priorities_;
  int ox_ = 0, oy_ = 0;
  bool xrepeat_ = true, yrepeat_ = true;
  int tilesize_ = 32;

  std::vector<TileQuad> ground_cache_;
  std::vector<std::vector<TileQuad>> aboves_cache_;
};

}  // namespace lime
