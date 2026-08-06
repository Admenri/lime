// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "src/font/font_atlas.h"
#include "src/font/glyph_cache.h"
#include "src/font/text_shaper.h"
#include "src/raywarp.h"

namespace rgssx::font {

class FontManager;

// ---------------------------------------------------------------------------
// Composes shaping + fallback segmentation + glyph cache + atlas to draw
// text on screen.
//
// DrawText flow:
//   1. Segment the UTF-8 string into per-font runs (fallback).
//   2. Shape each run with HarfBuzz.
//   3. Resolve every glyph through the GlyphCache (generates + uploads any
//      missing glyph into the atlas).
//   4. Advance a pen using the shaped advances and emit quads via
//      raylib::DrawTextureRec, using the pen + GPOS offset + glyph bearings.
// ---------------------------------------------------------------------------
class TextRenderer {
 public:
  TextRenderer(FontManager& manager, TextShaper& shaper, GlyphCache& cache,
               FontAtlas& atlas);

  // Draw UTF-8 text. `position` is the top-left of the first line box.
  // Lines are separated by '\n'. `color` is the tint (premultiplied pipeline:
  // opaque colors are the common case; for translucent tints premultiply the
  // RGB channels by alpha/255 first).
  void DrawText(const char* utf8, raylib::Vector2 position,
                raylib::Color color);

  // Bounding box of a possibly multi-line string: (max line width, total
  // line height).
  raylib::Vector2 MeasureText(const char* utf8);

 private:
  FontManager& manager_;
  TextShaper& shaper_;
  GlyphCache& cache_;
  FontAtlas& atlas_;
};

}  // namespace rgssx::font
