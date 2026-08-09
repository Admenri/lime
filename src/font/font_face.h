// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <hb.h>
#include <hb-ft.h>

namespace lime::font {

// ---------------------------------------------------------------------------
// One font file loaded at a fixed pixel size.
//
// Encapsulates the FreeType face + the matching HarfBuzz font so the rest of
// the pipeline never deals with raw library handles. RAII: the FT_Face and
// the hb_font_t are released in the destructor (hb first, since it references
// the FT face).
// ---------------------------------------------------------------------------
class FontFace {
 public:
  FontFace() = default;
  ~FontFace();

  FontFace(const FontFace&) = delete;
  FontFace& operator=(const FontFace&) = delete;
  FontFace(FontFace&& other) noexcept;
  FontFace& operator=(FontFace&& other) noexcept;

  // Load a TTF/OTF/TTC font. Tries the plain OS path first, then falls back
  // to the engine virtual file system (packed archives / RTP).
  bool Load(const char* filename, float pixel_size);

  bool IsValid() const { return face_ != nullptr; }

  FT_Face GetFTFace() const { return face_; }
  hb_font_t* GetHBFont() const { return hb_font_; }

  // Metrics in pixels, computed at the fixed size this face was loaded with.
  float GetAscent() const;
  float GetDescent() const;  // positive value (distance below the baseline)
  float GetLineHeight() const;
  float GetPixelSize() const { return pixel_size_; }
  const std::string& GetFilename() const { return filename_; }

  // True when the face has a glyph for `codepoint` (used by fallback).
  bool HasCodepoint(uint32_t codepoint) const;

  // Synthetic style applied at rasterization time via FreeType synthesis.
  void SetStyle(bool bold, bool italic);
  bool GetBold() const { return bold_; }
  bool GetItalic() const { return italic_; }

 private:
  void Reset();

  FT_Face face_ = nullptr;
  hb_font_t* hb_font_ = nullptr;
  std::vector<uint8_t> memory_;  // keeps font bytes alive for memory faces
  float pixel_size_ = 0.f;
  std::string filename_;
  bool bold_ = false;
  bool italic_ = false;
};

}  // namespace lime::font
