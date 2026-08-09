// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "src/raywarp.h"

namespace lime::font {

// ---------------------------------------------------------------------------
// Dynamic GPU glyph atlas.
//
// - Multiple 2048x2048 pages (created on demand).
// - Skyline packing per page.
// - Glyphs are uploaded incrementally with raylib::UpdateTextureRec() so only
//   the newly written region is pushed to the GPU.
// - Evicted glyphs leave an exact-size hole that is reused for later glyphs
//   of the same dimensions (very common at a fixed pixel size).
// ---------------------------------------------------------------------------
class FontAtlas {
 public:
  // A rectangle inside one page. GlyphCache pads bitmaps by 1px on every
  // side, so `width/height` include the border.
  struct Allocation {
    int page = -1;
    int x = 0, y = 0;
    int width = 0, height = 0;
  };

  // `premultiplied_alpha` selects the alpha convention of the uploaded glyph
  // pixels: premultiplied RGBA (default, matches raylib's font pipeline and
  // BLEND_ALPHA_PREMULTIPLY) or straight alpha (for plain BLEND_ALPHA).
  FontAtlas(int page_width = 2048, int page_height = 2048, int max_pages = 8,
            bool premultiplied_alpha = true);
  ~FontAtlas();

  FontAtlas(const FontAtlas&) = delete;
  FontAtlas& operator=(const FontAtlas&) = delete;

  // Reserve a region; creates a new page when needed. Returns false when the
  // atlas is full and cannot grow any further.
  bool Allocate(int width, int height, Allocation& out);

  // Reclaim a region (glyph eviction). Clears it to transparent and makes it
  // reusable for same-sized allocations.
  void Free(const Allocation& alloc);

  // Upload premultiplied RGBA8 (row-major, `src_pitch` bytes per row).
  void UploadRGBA(const Allocation& alloc, const unsigned char* rgba,
                  int src_width, int src_height, int src_pitch);

  // Upload a FreeType grayscale bitmap converted to premultiplied white
  // RGBA. The bitmap is placed 1px inside the allocation (transparent border
  // prevents bilinear bleeding between adjacent glyphs).
  void UploadGray(const Allocation& alloc, const unsigned char* gray,
                  int src_width, int src_height, int src_pitch);

  // Upload a FreeType color glyph (FT_PIXEL_MODE_BGRA, straight alpha)
  // converted to premultiplied RGBA.
  void UploadColor(const Allocation& alloc, const unsigned char* bgra,
                   int src_width, int src_height, int src_pitch);

  raylib::Texture2D GetTexture(int page) const;
  int PageCount() const { return static_cast<int>(pages_.size()); }
  int MaxPages() const { return max_pages_; }
  int PageWidth() const { return page_width_; }
  int PageHeight() const { return page_height_; }

  // Alpha convention of the stored glyphs. Toggling this invalidates existing
  // glyph data, so prefer FontSystem::SetPremultipliedAlpha() (which also
  // clears the cache) over setting it directly after glyphs were uploaded.
  bool IsPremultipliedAlpha() const { return premultiplied_alpha_; }
  void SetPremultipliedAlpha(bool value) { premultiplied_alpha_ = value; }

  // Fraction [0, 1] of a page's pixels that are currently in use.
  float Usage(int page) const;

  // Zero every page (used by FontSystem::ClearCache()).
  void Clear();

 private:
  struct Page;
  Page* CreatePage();
  void UploadRegion(Page& page, int x, int y, int w, int h,
                    const unsigned char* rgba);

  int page_width_;
  int page_height_;
  int max_pages_;
  bool premultiplied_alpha_ = true;
  std::vector<std::unique_ptr<Page>> pages_;
};

}  // namespace lime::font
