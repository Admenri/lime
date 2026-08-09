// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/font/font_system.h"

namespace lime::font {

FontSystem::FontSystem()
    : glyph_cache_(atlas_),
      renderer_(font_manager_, shaper_, glyph_cache_, atlas_) {}

FontSystem::~FontSystem() = default;

int FontSystem::LoadFont(const char* filename, float pixel_size) {
  return font_manager_.LoadFont(filename, pixel_size);
}

int FontSystem::LoadFallbackFont(const char* filename, float pixel_size) {
  return font_manager_.LoadFallback(filename, pixel_size);
}

void FontSystem::DrawText(const char* text, raylib::Vector2 position,
                          raylib::Color color) {
  renderer_.DrawText(text, position, color);
}

raylib::Vector2 FontSystem::MeasureText(const char* text) {
  return renderer_.MeasureText(text);
}

void FontSystem::ClearCache() { glyph_cache_.Clear(); }

void FontSystem::Trim(size_t count) { glyph_cache_.EvictLRU(count); }

void FontSystem::SetPremultipliedAlpha(bool value) {
  if (atlas_.IsPremultipliedAlpha() == value) return;
  atlas_.SetPremultipliedAlpha(value);
  // Glyphs already in the atlas were uploaded with the old alpha convention;
  // drop them so they are re-rasterized in the new format on the next draw.
  glyph_cache_.Clear();
}

}  // namespace lime::font
