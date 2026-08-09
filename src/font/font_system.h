// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <cstddef>

#include "src/font/font_atlas.h"
#include "src/font/font_manager.h"
#include "src/font/glyph_cache.h"
#include "src/font/text_renderer.h"
#include "src/font/text_shaper.h"
#include "src/raywarp.h"

namespace lime::font {

// ---------------------------------------------------------------------------
// Top-level font system: a drop-in replacement for raylib's static Font.
//
//   FontSystem fs;
//   fs.LoadFont("NotoSansCJK-Regular.ttf");        // primary (CJK + Latin)
//   fs.LoadFallbackFont("NotoColorEmoji.ttf");     // emoji fallback
//
//   while (!raylib::WindowShouldClose()) {
//     raylib::BeginDrawing();
//     fs.DrawText("你好世界 Hello World 😀", {100, 100}, raylib::WHITE);
//     raylib::EndDrawing();
//   }
//
// Design notes:
//   - Glyphs are rasterized lazily (only ASCII at first; CJK/emoji are
//     generated on first use), so huge character sets never preload.
//   - Same glyph is generated exactly once, and only new atlas regions are
//     pushed to the GPU (UpdateTextureRec).
//   - When the atlas runs out of pages the least-recently-used glyphs are
//     evicted (see FontSystem::Trim / ClearCache).
// ---------------------------------------------------------------------------
class FontSystem {
 public:
  FontSystem();
  ~FontSystem();

  FontSystem(const FontSystem&) = delete;
  FontSystem& operator=(const FontSystem&) = delete;

  // ---- Font management ------------------------------------------------
  // First successful load becomes the primary font. Returns the font index
  // or -1 on failure. Load fallbacks at the same pixel size for consistent
  // line metrics.
  int LoadFont(const char* filename, float pixel_size = 24.f);
  int LoadFallbackFont(const char* filename, float pixel_size = 24.f);

  // ---- Text rendering --------------------------------------------------
  // Draw UTF-8 text at `position` (top-left of the first line). Supports
  // CJK / Latin / Arabic (RTL) / emoji via the fallback chain, and '\n'
  // line breaks.
  void DrawText(const char* text, raylib::Vector2 position,
                raylib::Color color);

  // Bounding box of a possibly multi-line string.
  raylib::Vector2 MeasureText(const char* text);

  // ---- Cache control ---------------------------------------------------
  // Drop every cached glyph and reset the atlas (e.g. on a scene change or
  // when memory pressure is high). Glyphs regenerate on demand.
  void ClearCache();

  // Evict the `count` least-recently-used glyphs.
  void Trim(size_t count = 256);

  // Toggle the alpha convention of the glyph atlas. Premultiplied (default)
  // is what BLEND_ALPHA_PREMULTIPLY expects and makes the DrawText texture
  // premultiplied; setting it to false stores straight alpha and DrawText
  // switches to plain alpha blending. Switching clears the glyph cache so
  // glyphs re-rasterize in the new format.
  void SetPremultipliedAlpha(bool value);
  bool IsPremultipliedAlpha() const { return atlas_.IsPremultipliedAlpha(); }

  size_t CachedGlyphs() const { return glyph_cache_.GlyphCount(); }
  int AtlasPages() const { return atlas_.PageCount(); }

  // ---- Accessors (advanced use) ----------------------------------------
  FontManager& Fonts() { return font_manager_; }
  GlyphCache& Glyphs() { return glyph_cache_; }
  FontAtlas& Atlas() { return atlas_; }

 private:
  // Declaration order matters: each object is constructed after the ones it
  // references.
  FontManager font_manager_;
  TextShaper shaper_;
  FontAtlas atlas_;
  GlyphCache glyph_cache_;
  TextRenderer renderer_;
};

}  // namespace lime::font
