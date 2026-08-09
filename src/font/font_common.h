// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace lime::font {

class FontFace;

// ---------------------------------------------------------------------------
// One shaped glyph output from HarfBuzz. All values are in pixels.
// ---------------------------------------------------------------------------
struct GlyphPosition {
  uint32_t glyph_id = 0;
  uint32_t cluster = 0;  // UTF-8 byte offset of the source codepoint
  float x_advance = 0.f; // pen advance (x)
  float y_advance = 0.f; // pen advance (y, vertical text / diacritics)
  float x_offset = 0.f;  // glyph placement offset (GPOS)
  float y_offset = 0.f;
};

// ---------------------------------------------------------------------------
// Result of shaping one text run against one font.
// ---------------------------------------------------------------------------
struct GlyphRun {
  std::vector<GlyphPosition> glyphs;
  float advance_x = 0.f;  // total horizontal pen advance (pixels)
  float advance_y = 0.f;  // total vertical pen advance (pixels)
};

// ---------------------------------------------------------------------------
// A run of text assigned to a concrete face by the fallback segmenter.
// `text` points into the original UTF-8 buffer; `length` is in bytes.
// ---------------------------------------------------------------------------
struct TextSegment {
  FontFace* face = nullptr;
  const char* text = nullptr;
  size_t length = 0;
};

// ---------------------------------------------------------------------------
// UTF-8 helpers
// ---------------------------------------------------------------------------

// Decode a single UTF-8 codepoint at `s` and advance `s` past it.
// Invalid bytes are replaced with U+FFFD. Safe for NUL-terminated strings:
// the continuation-byte checks reject any byte < 0x80 (including the NUL
// terminator and '\n'), so we never read past the string terminator.
inline uint32_t DecodeUTF8(const char*& s) {
  const uint8_t* p = reinterpret_cast<const uint8_t*>(s);
  uint32_t cp = 0xFFFD;

  if (p[0] < 0x80) {
    cp = p[0];
    s += 1;
    return cp;
  }
  if ((p[0] & 0xE0u) == 0xC0u && p[1] != 0 && (p[1] & 0xC0u) == 0x80u) {
    cp = ((p[0] & 0x1Fu) << 6) | (p[1] & 0x3Fu);
    s += 2;
    return cp;
  }
  if ((p[0] & 0xF0u) == 0xE0u && p[1] != 0 && (p[1] & 0xC0u) == 0x80u &&
      p[2] != 0 && (p[2] & 0xC0u) == 0x80u) {
    cp = ((p[0] & 0x0Fu) << 12) | ((p[1] & 0x3Fu) << 6) | (p[2] & 0x3Fu);
    s += 3;
    return cp;
  }
  if ((p[0] & 0xF8u) == 0xF0u && p[1] != 0 && (p[1] & 0xC0u) == 0x80u &&
      p[2] != 0 && (p[2] & 0xC0u) == 0x80u && p[3] != 0 &&
      (p[3] & 0xC0u) == 0x80u) {
    cp = ((p[0] & 0x07u) << 18) | ((p[1] & 0x3Fu) << 12) |
         ((p[2] & 0x3Fu) << 6) | (p[3] & 0x3Fu);
    s += 4;
    return cp;
  }

  s += 1;  // invalid leading byte
  return cp;
}

}  // namespace lime::font
