// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/font/glyph_cache.h"

#include <algorithm>
#include <utility>
#include <vector>

#include <ft2build.h>
#include FT_SYNTHESIS_H

namespace rgssx::font {

GlyphCache::GlyphCache(FontAtlas& atlas) : atlas_(atlas) {}

bool GlyphCache::HasGlyph(const FontFace& face, uint32_t glyph_id) const {
  return glyphs_.find(Key{&face, glyph_id}) != glyphs_.end();
}

Glyph& GlyphCache::GetGlyph(FontFace& face, uint32_t glyph_id) {
  auto it = glyphs_.find(Key{&face, glyph_id});
  if (it != glyphs_.end()) {
    Touch(it->second);
    return it->second;
  }
  return AddGlyph(face, glyph_id);
}

Glyph& GlyphCache::AddGlyph(FontFace& face, uint32_t glyph_id) {
  auto [it, inserted] = glyphs_.try_emplace(Key{&face, glyph_id});
  if (!inserted) {
    Touch(it->second);
    return it->second;
  }
  it->second = Rasterize(face, glyph_id);
  Touch(it->second);
  return it->second;
}

// ---------------------------------------------------------------------------
// Core rasterizer: FT_Load_Glyph -> FT bitmap -> atlas upload.
// ---------------------------------------------------------------------------
Glyph GlyphCache::Rasterize(FontFace& face, uint32_t glyph_id) {
  Glyph out;
  out.face = &face;
  out.id = glyph_id;

  FT_Face ft = face.GetFTFace();
  if (!ft) return out;

  const bool bold = face.GetBold();
  const bool italic = face.GetItalic();
  const bool has_color = FT_HAS_COLOR(ft) != 0;

  FT_GlyphSlot slot = nullptr;
  bool loaded = false;

  if ((bold || italic) && !has_color) {
    // Synthetic style: load the outline (no FT_LOAD_RENDER), apply FreeType
    // synthesis (oblique / embolden), then render. Color-bitmap fonts (emoji)
    // are skipped because embedded color strikes cannot be synthesized.
    const int outline_flags = FT_LOAD_TARGET_NORMAL | FT_LOAD_NO_HINTING;
    if (FT_Load_Glyph(ft, glyph_id, outline_flags) == 0) {
      slot = ft->glyph;
      if (italic) FT_GlyphSlot_Oblique(slot);
      if (bold) FT_GlyphSlot_Embolden(slot);
      loaded = FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL) == 0;
    }
  }
  if (!loaded) {
    // Plain / color-aware path: prefer color bitmaps (CBDT/CBLC emoji) and
    // keep rasterization consistent with the no-hinting HarfBuzz advances.
    int flags =
        FT_LOAD_RENDER | FT_LOAD_TARGET_NORMAL | FT_LOAD_NO_HINTING | FT_LOAD_COLOR;
    if (FT_Load_Glyph(ft, glyph_id, flags) != 0) {
      flags &= ~FT_LOAD_COLOR;
      if (FT_Load_Glyph(ft, glyph_id, flags) != 0) return out;
    }
    slot = ft->glyph;
  }

  FT_Bitmap& bmp = slot->bitmap;

  // Metrics. Advance is 26.6 fixed point; bitmap placement uses the rendered
  // bitmap's left/top offsets (correct even after synthesis).
  out.advance = slot->metrics.horiAdvance / 64.0f;
  out.bearing_x = static_cast<float>(slot->bitmap_left);
  out.bearing_y = static_cast<float>(slot->bitmap_top);

  const int w = bmp.width;
  const int h = bmp.rows;
  if (w <= 0 || h <= 0)
    return out;  // no bitmap (space, combining marks): cache metrics only

  // Reserve a padded region (1px transparent border prevents bilinear bleed).
  FontAtlas::Allocation alloc;
  if (!atlas_.Allocate(w + 2, h + 2, alloc)) {
    // Atlas is full: evict the least-recently-used glyphs and retry once.
    EvictLRU(kEvictBatch);
    if (!atlas_.Allocate(w + 2, h + 2, alloc)) return out;
  }

  switch (bmp.pixel_mode) {
    case FT_PIXEL_MODE_GRAY:
      atlas_.UploadGray(alloc, bmp.buffer, w, h, bmp.pitch);
      break;
    case FT_PIXEL_MODE_BGRA:
      atlas_.UploadColor(alloc, bmp.buffer, w, h, bmp.pitch);
      break;
    case FT_PIXEL_MODE_MONO: {
      // 1-bit bitmap -> grayscale.
      std::vector<uint8_t> gray(static_cast<size_t>(w) * h, 0);
      for (int row = 0; row < h; ++row) {
        const uint8_t* src = bmp.buffer + static_cast<size_t>(row) * bmp.pitch;
        uint8_t* dst = gray.data() + static_cast<size_t>(row) * w;
        for (int col = 0; col < w; ++col)
          dst[col] = (src[col >> 3] & (0x80u >> (col & 7))) ? 255u : 0u;
      }
      atlas_.UploadGray(alloc, gray.data(), w, h, w);
      break;
    }
    default:
      atlas_.UploadGray(alloc, bmp.buffer, w, h, bmp.pitch);
      break;
  }

  out.page = alloc.page;
  out.atlas_rect = {static_cast<float>(alloc.x + 1), static_cast<float>(alloc.y + 1),
                    static_cast<float>(w), static_cast<float>(h)};
  return out;
}

void GlyphCache::EvictLRU(size_t count) {
  if (glyphs_.empty() || count == 0) return;

  // Sort cached glyphs by recency (oldest first).
  std::vector<std::pair<uint64_t, Key>> order;
  order.reserve(glyphs_.size());
  for (const auto& kv : glyphs_) order.emplace_back(kv.second.last_used, kv.first);
  std::sort(order.begin(), order.end(),
            [](const auto& a, const auto& b) { return a.first < b.first; });

  size_t evicted = 0;
  for (const auto& [last_used, key] : order) {
    if (evicted >= count) break;
    auto it = glyphs_.find(key);
    if (it == glyphs_.end()) continue;

    Glyph& g = it->second;
    if (g.page >= 0 && g.atlas_rect.width > 0.f && g.atlas_rect.height > 0.f) {
      // Reclaim the full padded region (content + 1px border).
      FontAtlas::Allocation alloc;
      alloc.page = g.page;
      alloc.x = static_cast<int>(g.atlas_rect.x) - 1;
      alloc.y = static_cast<int>(g.atlas_rect.y) - 1;
      alloc.width = static_cast<int>(g.atlas_rect.width) + 2;
      alloc.height = static_cast<int>(g.atlas_rect.height) + 2;
      atlas_.Free(alloc);
    }
    glyphs_.erase(it);
    ++evicted;
  }
}

void GlyphCache::Clear() {
  glyphs_.clear();
  atlas_.Clear();
}

}  // namespace rgssx::font
