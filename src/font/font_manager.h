// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "src/font/font_common.h"
#include "src/font/font_face.h"

namespace lime::font {

// ---------------------------------------------------------------------------
// Owns the font list (primary + fallback chain) and resolves which face
// covers a given codepoint. Fonts are tried in load order, so the first
// loaded font is the primary and later fonts act as fallbacks.
//
// Fallback example:
//   primary : NotoSansCJK   (covers CJK + Latin)
//   fallback: NotoColorEmoji (covers emoji)
//
// Segmentation assigns every codepoint to the first face that has it, then
// groups consecutive codepoints with the same face into TextSegments so each
// segment can be shaped independently.
// ---------------------------------------------------------------------------
class FontManager {
 public:
  // Load a font and append it to the chain. Returns its index (0 = primary)
  // or -1 on failure.
  int LoadFont(const char* filename, float pixel_size = 24.f);
  int LoadFallback(const char* filename, float pixel_size = 24.f);

  FontFace* GetFont(int index) const;
  int FontCount() const { return static_cast<int>(fonts_.size()); }

  // First face in the chain with a glyph for `codepoint`. Falls back to the
  // primary face when nothing covers it (so shaping still produces a
  // .notdef glyph instead of skipping).
  FontFace* ResolveFont(uint32_t codepoint) const;

  // Split UTF-8 text into per-font runs for fallback shaping. `text` in the
  // returned segments points into the original buffer.
  std::vector<TextSegment> Segment(const char* utf8) const;
  std::vector<TextSegment> Segment(const char* utf8, size_t length) const;

 private:
  std::vector<std::unique_ptr<FontFace>> fonts_;
};

}  // namespace lime::font
