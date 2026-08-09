// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <cstddef>

#include <hb.h>

#include "src/font/font_common.h"
#include "src/font/font_face.h"

namespace lime::font {

// ---------------------------------------------------------------------------
// Shapes UTF-8 text into a GlyphRun using HarfBuzz.
//
// Handles OpenType GSUB (ligatures, substitution) and GPOS (kerning, mark
// positioning), plus complex scripts (Arabic shaping, Hangul, ...). The
// script/direction/language are guessed from the text itself, so CJK, RTL
// Arabic and Latin all work without extra input.
// ---------------------------------------------------------------------------
class TextShaper {
 public:
  // Shape `length` bytes of UTF-8 with `font`. `direction` overrides the
  // direction guessed by hb_buffer_guess_segment_properties() (pass
  // HB_DIRECTION_INVALID to keep the guess).
  GlyphRun Shape(FontFace& font, const char* utf8, size_t length,
                 hb_direction_t direction = HB_DIRECTION_INVALID) const;

  // Convenience for NUL-terminated strings.
  GlyphRun Shape(FontFace& font, const char* utf8) const;
};

}  // namespace lime::font
