// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Admenri Adev <admenri0504@gmail.com>.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the “Software”), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "src/font.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <memory>
#include <unordered_map>
#include <utility>

#include "src/filesystem.h"
#include "src/font/font_system.h"

namespace lime {
namespace {

using font::FontSystem;

// ---------------------------------------------------------------------------
// Dynamic font system sharing.
//
// Every distinct (name chain, size, style) combination gets one shared
// font::FontSystem (which owns the GPU glyph atlas + cache). Font instances
// with the same configuration share it, so the default font used by every
// Bitmap only allocates a single atlas. The weak_ptr cache releases the GPU
// resources once the last Font using them is destroyed.
// ---------------------------------------------------------------------------
struct FontSystemKey {
  std::vector<std::string> names;
  int size = 0;
  bool bold = false;
  bool italic = false;

  bool operator==(const FontSystemKey& o) const {
    return size == o.size && bold == o.bold && italic == o.italic &&
           names == o.names;
  }
};

struct FontSystemKeyHash {
  size_t operator()(const FontSystemKey& k) const {
    size_t h = std::hash<int>{}(k.size);
    h = h * 0x9E3779B1u + std::hash<bool>{}(k.bold);
    h = h * 0x9E3779B1u + std::hash<bool>{}(k.italic);
    for (const auto& n : k.names)
      h = h * 0x9E3779B1u + std::hash<std::string>{}(n);
    return h;
  }
};

std::unordered_map<FontSystemKey, std::weak_ptr<FontSystem>, FontSystemKeyHash>
    g_font_systems;

// ---------------------------------------------------------------------------
// RGSS font name -> font file resolution.
// ---------------------------------------------------------------------------
bool FontFileExists(const std::string& path) {
  FILE* f = nullptr;
  if (fopen_s(&f, path.c_str(), "rb") == 0 && f) {
    fclose(f);
    return true;
  }
  if (IOService::Instance() && IOService::Instance()->Exists(path))
    return true;
  return false;
}

std::string ToLower(std::string s) {
  for (char& c : s)
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  return s;
}

std::string SystemFontsDir() {
  const char* windir = std::getenv("WINDIR");
  std::string dir = windir ? windir : "C:/Windows";
  return dir + "/Fonts/";
}

const char* const kFontExtensions[] = {"ttf", "ttc", "otf", "otc"};

// Resolve a font name (a filename like "SimHei.ttf") to a loadable path.
// Priority:
//   1. <program dir>/Fonts/<name> via the engine virtual file system (the
//      app mounts the executable directory at "/", so "Fonts/..." resolves
//      to <program dir>/Fonts/...).
//   2. The bare name (relative path / already inside the virtual FS).
//   3. The system fonts directory (convenience for system fonts).
std::string ResolveFontFile(const std::string& name) {
  const std::string lower_name = ToLower(name);
  const bool has_ext = lower_name.find('.') != std::string::npos;

  // 1) Game <program dir>/Fonts/ directory. Match the requested filename
  //    case-insensitively through the virtual file system, so it works even
  //    when the on-disk case differs (e.g. "simhei.ttf" vs "SimHei.ttf") or
  //    the working directory is not the game directory.
  if (IOService::Instance()) {
    auto files = IOService::Instance()->EnumDir("Fonts");
    for (const auto& f : files) {
      std::string stem = f;
      const size_t dot = stem.find_last_of('.');
      if (dot != std::string::npos)
        stem = stem.substr(0, dot);
      if (ToLower(f) == lower_name || ToLower(stem) == lower_name)
        return "Fonts/" + f;
    }
  }
  std::vector<std::string> stems = {"Fonts/" + name};
  if (!has_ext)
    for (const char* ext : kFontExtensions)
      stems.push_back("Fonts/" + name + "." + ext);
  for (const auto& s : stems)
    if (FontFileExists(s))
      return s;

  // 2) Bare name.
  if (FontFileExists(name))
    return name;

  // 3) System fonts directory.
  const std::string fonts_dir = SystemFontsDir();
  std::vector<std::string> sys_stems = {fonts_dir + name};
  if (!has_ext)
    for (const char* ext : kFontExtensions)
      sys_stems.push_back(fonts_dir + name + "." + ext);
  for (const auto& s : sys_stems)
    if (FontFileExists(s))
      return s;

  return {};
}

// Last-resort default font when none of the requested names resolve.
std::string ResolveDefaultFontFile() {
  static const char* const kDefaults[] = {
      "msyh.ttc", "simhei.ttf", "simsun.ttc", "arial.ttf", "segoeui.ttf",
  };
  // Game Fonts/ directory first, then the system fonts directory.
  for (const char* f : kDefaults) {
    std::string in_fonts = std::string("Fonts/") + f;
    if (FontFileExists(in_fonts))
      return in_fonts;
  }
  const std::string fonts_dir = SystemFontsDir();
  for (const char* f : kDefaults)
    if (FontFileExists(fonts_dir + f))
      return fonts_dir + f;
  return {};
}

}  // namespace

Font::Font(std::vector<std::string> names, int size)
    : name_(names),
      size_(size),
      bold_(Attr_DefaultBold()),
      italic_(Attr_DefaultItalic()),
      outline_(Attr_DefaultOutline()),
      shadow_(Attr_DefaultShadow()),
      color_(*Attr_DefaultColor()),
      out_color_(*Attr_DefaultOutColor()) {
  // When no name is given, use Font.default_name (RGSS behaviour).
  if (name_.empty()) {
    auto default_name = Attr_DefaultName();
    if (default_name.has_value() && !default_name->empty())
      name_ = *default_name;
  }
}

Font::Font(RefPtr<Font> other)
    : name_(other->name_),
      size_(other->size_),
      bold_(other->bold_),
      italic_(other->italic_),
      outline_(other->outline_),
      shadow_(other->shadow_),
      color_(other->color_),
      out_color_(other->out_color_) {}

Font::~Font() = default;

void Font::ResetFontSystem() {
  font_system_.reset();
}

// static
bool Font::Exist(std::string name) {
  return !ResolveFontFile(name).empty();
}

font::FontSystem& Font::font_system() {
  // Instance cache hit.
  if (font_system_)
    return *font_system_;

  // The default font (empty name chain) falls back to the RGSS defaults.
  std::vector<std::string> names = name_;
  if (names.empty()) {
    auto defaults = Attr_DefaultName();
    if (defaults.has_value() && !defaults->empty())
      names = *defaults;
  }
  const int size = std::max(size_, 1);

  FontSystemKey key{names, size, bold_, italic_};
  auto it = g_font_systems.find(key);
  if (it != g_font_systems.end()) {
    if (auto sys = it->second.lock()) {
      font_system_ = std::move(sys);
      return *font_system_;
    }
    g_font_systems.erase(it);  // stale entry
  }

  auto sys = std::make_shared<FontSystem>();
  bool loaded = false;
  for (const auto& name : names) {
    if (name.empty())
      continue;
    std::string file = ResolveFontFile(name);
    if (file.empty())
      continue;
    const int index = sys->LoadFont(file.c_str(), static_cast<float>(size));
    if (index >= 0) {
      if (auto* face = sys->Fonts().GetFont(index))
        face->SetStyle(bold_, italic_);
      loaded = true;
    }
  }
  if (!loaded) {
    std::string file = ResolveDefaultFontFile();
    if (!file.empty()) {
      const int index = sys->LoadFont(file.c_str(), static_cast<float>(size));
      if (index >= 0)
        if (auto* face = sys->Fonts().GetFont(index))
          face->SetStyle(bold_, italic_);
    }
  }

  if (sys->Fonts().FontCount() == 0) {
    std::string name_str;
    for (const auto& n : names) {
      if (!name_str.empty())
        name_str += ", ";
      name_str += n;
    }
    if (name_str.empty())
      name_str = "(default)";
    raylib::TraceLog(raylib::LOG_WARNING,
                     "Font: no font face loaded (names: %s, size: %d); "
                     "check the file in <game>/Fonts or the system fonts",
                     name_str.c_str(), size);
  }

  g_font_systems.emplace(key, sys);
  font_system_ = std::move(sys);
  return *font_system_;
}

ATTR_DEF(std::vector<std::string>, Name, Font) {
  if (value.has_value()) {
    name_ = *value;
    ResetFontSystem();
    return std::nullopt;
  } else {
    return name_;
  }
}

ATTR_DEF(int, Size, Font) {
  if (value.has_value()) {
    size_ = *value;
    ResetFontSystem();
    return std::nullopt;
  } else {
    return size_;
  }
}

ATTR_DEF(bool, Bold, Font) {
  if (value.has_value()) {
    bold_ = *value;
    ResetFontSystem();
    return std::nullopt;
  } else {
    return bold_;
  }
}

ATTR_DEF(bool, Italic, Font) {
  if (value.has_value()) {
    italic_ = *value;
    ResetFontSystem();
    return std::nullopt;
  } else {
    return italic_;
  }
}

ATTR_DEF(bool, Outline, Font) {
  if (value.has_value()) {
    outline_ = *value;
    return std::nullopt;
  } else {
    return outline_;
  }
}

ATTR_DEF(bool, Shadow, Font) {
  if (value.has_value()) {
    shadow_ = *value;
    return std::nullopt;
  } else {
    return shadow_;
  }
}

ATTR_DEF(RefPtr<Color>, Color, Font) {
  if (value.has_value()) {
    color_ = *value;
    return std::nullopt;
  } else {
    return color_;
  }
}

ATTR_DEF(RefPtr<Color>, OutColor, Font) {
  if (value.has_value()) {
    out_color_ = *value;
    return std::nullopt;
  } else {
    return out_color_;
  }
}

// -----------------------------------------------------------

ATTR_DEF(std::vector<std::string>, DefaultName, Font) {
  static std::vector<std::string> default_names = {"Default.ttf"};
  if (value.has_value()) {
    default_names = *value;
    return std::nullopt;
  } else {
    return default_names;
  }
}

ATTR_DEF(int, DefaultSize, Font) {
  static int default_size = 24;
  if (value.has_value()) {
    default_size = *value;
    return std::nullopt;
  } else {
    return default_size;
  }
}

ATTR_DEF(bool, DefaultBold, Font) {
  static bool default_bold = false;
  if (value.has_value()) {
    default_bold = *value;
    return std::nullopt;
  } else {
    return default_bold;
  }
}

ATTR_DEF(bool, DefaultItalic, Font) {
  static bool default_italic = false;
  if (value.has_value()) {
    default_italic = *value;
    return std::nullopt;
  } else {
    return default_italic;
  }
}

ATTR_DEF(bool, DefaultOutline, Font) {
  static bool default_outline = true;
  if (value.has_value()) {
    default_outline = *value;
    return std::nullopt;
  } else {
    return default_outline;
  }
}

ATTR_DEF(bool, DefaultShadow, Font) {
  static bool default_shadow = false;
  if (value.has_value()) {
    default_shadow = *value;
    return std::nullopt;
  } else {
    return default_shadow;
  }
}

ATTR_DEF(RefPtr<Color>, DefaultColor, Font) {
  static RefPtr<Color> default_color =
      MakeRefCounted<Color>(255, 255, 255, 255);
  if (value.has_value()) {
    default_color = *value;
    return std::nullopt;
  } else {
    return default_color;
  }
}

ATTR_DEF(RefPtr<Color>, DefaultOutColor, Font) {
  static RefPtr<Color> default_out_color = MakeRefCounted<Color>(0, 0, 0, 128);
  if (value.has_value()) {
    default_out_color = *value;
    return std::nullopt;
  } else {
    return default_out_color;
  }
}

}  // namespace lime
