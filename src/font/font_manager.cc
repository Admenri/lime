// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/font/font_manager.h"

#include <cstring>
#include <utility>

namespace rgssx::font {

int FontManager::LoadFont(const char* filename, float pixel_size) {
  auto face = std::make_unique<FontFace>();
  if (!face->Load(filename, pixel_size)) return -1;
  fonts_.push_back(std::move(face));
  return static_cast<int>(fonts_.size()) - 1;
}

int FontManager::LoadFallback(const char* filename, float pixel_size) {
  return LoadFont(filename, pixel_size);
}

FontFace* FontManager::GetFont(int index) const {
  if (index < 0 || index >= static_cast<int>(fonts_.size())) return nullptr;
  return fonts_[static_cast<size_t>(index)].get();
}

FontFace* FontManager::ResolveFont(uint32_t codepoint) const {
  for (const auto& face : fonts_)
    if (face->HasCodepoint(codepoint)) return face.get();
  // Nothing covers it: keep shaping with the primary (.notdef output).
  return fonts_.empty() ? nullptr : fonts_.front().get();
}

std::vector<TextSegment> FontManager::Segment(const char* utf8) const {
  return Segment(utf8, utf8 ? std::strlen(utf8) : 0);
}

std::vector<TextSegment> FontManager::Segment(const char* utf8,
                                              size_t length) const {
  std::vector<TextSegment> out;
  if (fonts_.empty() || !utf8 || length == 0) return out;

  const char* end = utf8 + length;
  const char* p = utf8;
  FontFace* seg_font = nullptr;
  const char* seg_start = nullptr;

  auto flush = [&]() {
    if (seg_font && seg_start)
      out.push_back({seg_font, seg_start, static_cast<size_t>(p - seg_start)});
    seg_font = nullptr;
    seg_start = nullptr;
  };

  while (p < end && *p) {
    const char* cp_start = p;
    const uint32_t cp = DecodeUTF8(p);
    if (p > end) {  // malformed tail crossing the range boundary
      p = end;
      break;
    }
    FontFace* font = ResolveFont(cp);
    if (font != seg_font) {
      flush();
      seg_font = font;
      seg_start = cp_start;
    }
  }
  flush();
  return out;
}

}  // namespace rgssx::font
