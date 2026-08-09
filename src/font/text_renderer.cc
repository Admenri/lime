// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/font/text_renderer.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "src/font/font_manager.h"

namespace lime::font {
namespace {

// Split a UTF-8 string into lines at '\n'. Returns (start, byte length)
// ranges that point into the original buffer.
void SplitLines(const char* utf8, std::vector<std::pair<const char*, size_t>>& lines) {
  const char* start = utf8;
  while (*utf8) {
    if (*utf8 == '\n') {
      lines.emplace_back(start, static_cast<size_t>(utf8 - start));
      start = utf8 + 1;
    }
    ++utf8;
  }
  lines.emplace_back(start, static_cast<size_t>(utf8 - start));
}

// Pen advance for a shaped glyph. Synthesized bold (FT_GlyphSlot_Embolden)
// grows the stored glyph advance, but HarfBuzz does not know about the
// synthesis, so bold glyphs advance by their (enlarged) stored advance.
float GlyphPenAdvance(const Glyph& g, const GlyphPosition& pos) {
  return (g.face && g.face->GetBold() && g.advance > 0.f) ? g.advance
                                                          : pos.x_advance;
}

}  // namespace

TextRenderer::TextRenderer(FontManager& manager, TextShaper& shaper,
                           GlyphCache& cache, FontAtlas& atlas)
    : manager_(manager), shaper_(shaper), cache_(cache), atlas_(atlas) {}

void TextRenderer::DrawText(const char* utf8, raylib::Vector2 position,
                            raylib::Color color) {
  if (!utf8 || !*utf8 || manager_.FontCount() == 0) return;

  std::vector<std::pair<const char*, size_t>> lines;
  SplitLines(utf8, lines);

  FontFace* primary = manager_.GetFont(0);
  if (!primary) return;
  const float ascent = primary->GetAscent();
  const float line_height = ascent + primary->GetDescent();

  // 1) Shape + resolve glyphs. This is where missing glyphs get rasterized
  //    and uploaded, so it must happen before we start emitting draw calls.
  struct Item {
    int line;
    const FontFace* face;
    const Glyph* glyph;
    GlyphPosition pos;
  };
  std::vector<Item> items;
  for (size_t li = 0; li < lines.size(); ++li) {
    for (const TextSegment& seg :
         manager_.Segment(lines[li].first, lines[li].second)) {
      if (!seg.face) continue;
      GlyphRun run = shaper_.Shape(*seg.face, seg.text, seg.length);
      for (const GlyphPosition& gp : run.glyphs)
        items.push_back({static_cast<int>(li), seg.face,
                         &cache_.GetGlyph(*seg.face, gp.glyph_id), gp});
    }
  }
  if (items.empty()) return;

  // 2) Emit quads, advancing the pen with HarfBuzz advances. Use the blend
  //    mode that matches the atlas glyphs' alpha convention.
  const int blend_mode = atlas_.IsPremultipliedAlpha()
                             ? raylib::BLEND_ALPHA_PREMULTIPLY
                             : raylib::BLEND_ALPHA;
  raylib::BeginBlendMode(blend_mode);
  {
    raylib::rlEnableColorBlend();

    float pen_x = position.x;
    float pen_y = position.y + ascent;
    int current_line = -1;
    for (const Item& item : items) {
      if (item.line != current_line) {
        current_line = item.line;
        pen_x = position.x;
        pen_y = position.y + ascent +
               static_cast<float>(current_line) * line_height;
      }

      const Glyph& g = *item.glyph;
      if (g.atlas_rect.width <= 0.f || g.atlas_rect.height <= 0.f) {
        // No bitmap (space / missing glyph): advance the pen only.
        pen_x += GlyphPenAdvance(g, item.pos);
        pen_y += item.pos.y_advance;
        continue;
      }

      // Top-left of the bitmap in screen space (Y down). HarfBuzz offsets and
      // FreeType bearings are Y-up, so they subtract from the baseline.
      const float sx = pen_x + item.pos.x_offset + g.bearing_x;
      const float sy = pen_y - (item.pos.y_offset + g.bearing_y);

      raylib::DrawTextureRec(atlas_.GetTexture(g.page), g.atlas_rect,
                             {sx, sy}, color);

      pen_x += GlyphPenAdvance(g, item.pos);
      pen_y += item.pos.y_advance;
    }
  }
  raylib::EndBlendMode();
}

raylib::Vector2 TextRenderer::MeasureText(const char* utf8) {
  raylib::Vector2 size = {};
  if (!utf8 || manager_.FontCount() == 0) return size;

  FontFace* primary = manager_.GetFont(0);
  if (!primary) return size;
  const float line_height = primary->GetAscent() + primary->GetDescent();

  std::vector<std::pair<const char*, size_t>> lines;
  SplitLines(utf8, lines);

  float max_width = 0.f;
  for (const auto& [text, length] : lines) {
    float line_width = 0.f;
    for (const TextSegment& seg : manager_.Segment(text, length)) {
      if (!seg.face) continue;
      GlyphRun run = shaper_.Shape(*seg.face, seg.text, seg.length);
      line_width += run.advance_x;
      if (seg.face->GetBold())
        // Synthesized bold adds ~size/16 px per glyph to the advance.
        line_width += run.glyphs.size() * (seg.face->GetPixelSize() / 16.0f);
    }
    max_width = std::max(max_width, line_width);
  }

  size.x = max_width;
  size.y = line_height * static_cast<float>(lines.size());
  return size;
}

}  // namespace lime::font
