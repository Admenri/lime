// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/font/text_shaper.h"

#include <cstring>

namespace lime::font {

GlyphRun TextShaper::Shape(FontFace& font, const char* utf8) const {
  return Shape(font, utf8, std::strlen(utf8));
}

GlyphRun TextShaper::Shape(FontFace& font, const char* utf8, size_t length,
                           hb_direction_t direction) const {
  GlyphRun run;
  hb_font_t* hb_font = font.GetHBFont();
  if (!hb_font || !utf8 || length == 0) return run;

  hb_buffer_t* buffer = hb_buffer_create();
  hb_buffer_add_utf8(buffer, utf8, static_cast<int>(length), 0,
                     static_cast<int>(length));
  hb_buffer_guess_segment_properties(buffer);
  if (direction != HB_DIRECTION_INVALID)
    hb_buffer_set_direction(buffer, direction);

  hb_shape(hb_font, buffer, nullptr, 0);

  unsigned int count = 0;
  hb_glyph_info_t* info = hb_buffer_get_glyph_infos(buffer, &count);
  hb_glyph_position_t* pos = hb_buffer_get_glyph_positions(buffer, nullptr);

  run.glyphs.reserve(count);
  for (unsigned int i = 0; i < count; ++i) {
    // hb-ft reports positions in 26.6 fixed point (1/64 pixel): the hb_font
    // scale is derived from the FT pixel size, which is 64 units per pixel.
    GlyphPosition gp;
    gp.glyph_id = info[i].codepoint;
    gp.cluster = info[i].cluster;
    gp.x_advance = pos[i].x_advance / 64.0f;
    gp.y_advance = pos[i].y_advance / 64.0f;
    gp.x_offset = pos[i].x_offset / 64.0f;
    gp.y_offset = pos[i].y_offset / 64.0f;
    run.advance_x += gp.x_advance;
    run.advance_y += gp.y_advance;
    run.glyphs.push_back(gp);
  }

  hb_buffer_destroy(buffer);
  return run;
}

}  // namespace lime::font
