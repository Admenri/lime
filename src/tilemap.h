#pragma once

#include "bitmap.h"
#include "common.h"
#include "refptr.h"
#include "table.h"
#include "utility.h"
#include "viewport.h"

namespace rgssx {

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
    raylib::Color color;
  };

  void DisposeObject() override;
  void Prepare() override;
  void Draw(DrawParam param) override;

  void MakeTilemapAtlas();
  void ParseMapData();
  void UpdateViewport(DrawParam param);

  void DrawLayer(const std::vector<TileQuad>& data);

  raylib::RenderTexture2D atlas_ = {};
  std::unique_ptr<TilemapAbove> above_;
  raylib::Rectangle render_viewport_ = {};
  raylib::Vector2 render_offset_ = {};
  raylib::Vector2 animation_offset_ = {};

  int flash_timer_ = 0;
  int flash_opacity_ = 0;
  int frame_index_ = 0;

  std::vector<TileQuad> ground_quads_;
  std::vector<TileQuad> above_quads_;

  RefPtr<Bitmap> bitmaps_[9] = {};
  RefPtr<Table> map_data_;
  RefPtr<Table> flash_data_;
  RefPtr<Table> flags_;
  int ox_ = 0, oy_ = 0;
  bool rgss3_style_ = true;
  bool xrepeat_ = true, yrepeat_ = true;
  int tilesize_ = 32;

  bool atlas_dirty_ = true;
};

}  // namespace rgssx
