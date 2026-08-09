// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <cstdint>
#include <functional>
#include <unordered_map>

#include "src/font/font_atlas.h"
#include "src/font/font_face.h"
#include "src/raywarp.h"

namespace lime::font {

// ---------------------------------------------------------------------------
// Cached glyph: atlas location + raster metrics for one (face, glyph id).
// ---------------------------------------------------------------------------
struct Glyph {
  uint32_t id = 0;
  const FontFace* face = nullptr;
  int page = -1;
  raylib::Rectangle atlas_rect = {};  // content rect (excluding 1px border)
  float advance = 0.f;                // horizontal advance (pixels)
  float bearing_x = 0.f;              // left side bearing (pixels)
  float bearing_y = 0.f;              // top side bearing (pixels)
  uint64_t last_used = 0;             // LRU clock
};

// ---------------------------------------------------------------------------
// (face, glyph id) -> Glyph cache.
//
// Glyphs are rasterized lazily on first use (CJK / emoji are generated only
// when the text actually needs them) and uploaded into the FontAtlas. When
// the atlas is full the least-recently-used glyphs are evicted.
// ---------------------------------------------------------------------------
class GlyphCache {
 public:
  explicit GlyphCache(FontAtlas& atlas);

  // True when the glyph is already cached.
  bool HasGlyph(const FontFace& face, uint32_t glyph_id) const;

  // Returns the cached glyph, generating it on demand.
  Glyph& GetGlyph(FontFace& face, uint32_t glyph_id);

  // Rasterizes the glyph (if missing) and returns it.
  Glyph& AddGlyph(FontFace& face, uint32_t glyph_id);

  // Evict `count` least-recently-used glyphs (frees atlas space).
  void EvictLRU(size_t count);

  // Evict every glyph and clear the atlas (used on cache reset).
  void Clear();

  size_t GlyphCount() const { return glyphs_.size(); }

 private:
  struct Key {
    const FontFace* face;
    uint32_t id;
    bool operator==(const Key& o) const {
      return face == o.face && id == o.id;
    }
  };
  struct KeyHash {
    size_t operator()(const Key& k) const {
      return (std::hash<const void*>{}(k.face) * 0x9E3779B97F4A7C15ull) ^ k.id;
    }
  };

  // Core FreeType rasterization (load, render, convert, upload).
  Glyph Rasterize(FontFace& face, uint32_t glyph_id);
  void Touch(Glyph& g) { g.last_used = ++clock_; }

  FontAtlas& atlas_;
  std::unordered_map<Key, Glyph, KeyHash> glyphs_;
  uint64_t clock_ = 0;

  static constexpr size_t kEvictBatch = 64;
};

}  // namespace lime::font
