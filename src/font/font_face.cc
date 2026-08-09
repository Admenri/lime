// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/font/font_face.h"

#include <cstring>
#include <utility>

#include "src/filesystem.h"

namespace lime::font {
namespace {

// ---------------------------------------------------------------------------
// Process-wide FreeType library shared by every FontFace (RAII singleton).
// FreeType recommends one library per application, and faces from the same
// library may share caches, so this is both cheaper and safer than creating
// a fresh library per face.
// ---------------------------------------------------------------------------
struct FreeTypeLibrary {
  FT_Library library = nullptr;

  FreeTypeLibrary() { FT_Init_FreeType(&library); }
  ~FreeTypeLibrary() {
    if (library) FT_Done_FreeType(library);
  }

  FreeTypeLibrary(const FreeTypeLibrary&) = delete;
  FreeTypeLibrary& operator=(const FreeTypeLibrary&) = delete;
};

FreeTypeLibrary& GetFreeTypeLibrary() {
  static FreeTypeLibrary instance;
  return instance;
}

}  // namespace

FontFace::~FontFace() { Reset(); }

FontFace::FontFace(FontFace&& other) noexcept
    : face_(other.face_),
      hb_font_(other.hb_font_),
      memory_(std::move(other.memory_)),
      pixel_size_(other.pixel_size_),
      filename_(std::move(other.filename_)),
      bold_(other.bold_),
      italic_(other.italic_) {
  other.face_ = nullptr;
  other.hb_font_ = nullptr;
}

FontFace& FontFace::operator=(FontFace&& other) noexcept {
  if (this != &other) {
    Reset();
    face_ = other.face_;
    hb_font_ = other.hb_font_;
    memory_ = std::move(other.memory_);
    pixel_size_ = other.pixel_size_;
    filename_ = std::move(other.filename_);
    bold_ = other.bold_;
    italic_ = other.italic_;
    other.face_ = nullptr;
    other.hb_font_ = nullptr;
  }
  return *this;
}

void FontFace::Reset() {
  // HarfBuzz must die before the FT face it references.
  if (hb_font_) {
    hb_font_destroy(hb_font_);
    hb_font_ = nullptr;
  }
  if (face_) {
    FT_Done_Face(face_);
    face_ = nullptr;
  }
  memory_.clear();
  pixel_size_ = 0.f;
  filename_.clear();
  bold_ = false;
  italic_ = false;
}

bool FontFace::Load(const char* filename, float pixel_size) {
  Reset();

  FT_Library library = GetFreeTypeLibrary().library;
  if (!library || !filename || pixel_size <= 0.f) return false;

  // 1) Plain OS path.
  FT_Error err = FT_New_Face(library, filename, 0, &face_);

  // 2) Engine virtual file system (packed archives / RTP). The font bytes
  //    are copied into `memory_` which outlives the face.
  if (err != 0 && IOService::Instance() &&
      IOService::Instance()->Exists(filename)) {
    if (auto stream = IOService::Instance()->OpenReadRaw(filename)) {
      std::string data = stream->ReadAll();
      if (!data.empty()) {
        err = FT_New_Memory_Face(
            library, reinterpret_cast<const FT_Byte*>(data.data()),
            static_cast<FT_Long>(data.size()), 0, &face_);
        if (err == 0)
          memory_ = std::vector<uint8_t>(data.begin(), data.end());
      }
    }
  }
  if (err != 0 || !face_) {
    Reset();
    return false;
  }

  // 3) Fixed pixel size. HarfBuzz derives its 26.6 scale from this, so it
  //    must be set before creating the hb_font.
  if (FT_Set_Pixel_Sizes(face_, 0, static_cast<FT_UInt>(pixel_size)) != 0) {
    Reset();
    return false;
  }

  // 4) HarfBuzz wrapper over the FT face. Positions reported by hb_shape
  //    come back in 1/64 px (26.6 fixed point).
  hb_font_ = hb_ft_font_create(face_, nullptr);
  if (!hb_font_) {
    Reset();
    return false;
  }
  // Keep shaping advances consistent with how GlyphCache rasterizes.
  hb_ft_font_set_load_flags(hb_font_, FT_LOAD_DEFAULT | FT_LOAD_NO_HINTING);

  filename_ = filename;
  pixel_size_ = pixel_size;
  return true;
}

float FontFace::GetAscent() const {
  return face_ ? face_->size->metrics.ascender / 64.0f : 0.f;
}

float FontFace::GetDescent() const {
  return face_ ? -face_->size->metrics.descender / 64.0f : 0.f;
}

float FontFace::GetLineHeight() const {
  return face_ ? face_->size->metrics.height / 64.0f : 0.f;
}

bool FontFace::HasCodepoint(uint32_t codepoint) const {
  return face_ &&
         FT_Get_Char_Index(face_, static_cast<FT_ULong>(codepoint)) != 0;
}

void FontFace::SetStyle(bool bold, bool italic) {
  bold_ = bold;
  italic_ = italic;
}

}  // namespace lime::font
