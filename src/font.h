#pragma once

#include "src/common.h"
#include "src/refptr.h"
#include "src/utility.h"

namespace lime {

namespace font {
class FontSystem;
}

class Font : public RefCounted<Font> {
 public:
  Font(std::vector<std::string> names = {}, int size = 24);
  Font(RefPtr<Font> other);
  ~Font();

  /*-export.begin-*/
  static bool Exist(std::string name);

  ATTR(std::vector<std::string>, Name);
  ATTR(int, Size);
  ATTR(bool, Bold);
  ATTR(bool, Italic);
  ATTR(bool, Outline);
  ATTR(bool, Shadow);
  ATTR(RefPtr<Color>, Color);
  ATTR(RefPtr<Color>, OutColor);

  static ATTR(std::vector<std::string>, DefaultName);
  static ATTR(int, DefaultSize);
  static ATTR(bool, DefaultBold);
  static ATTR(bool, DefaultItalic);
  static ATTR(bool, DefaultOutline);
  static ATTR(bool, DefaultShadow);
  static ATTR(RefPtr<Color>, DefaultColor);
  static ATTR(RefPtr<Color>, DefaultOutColor);
  /*-export.end-*/

 public:
  // Lazy-built dynamic font system (FreeType + HarfBuzz glyph atlas). Shared
  // by every Font with the same (name chain, size, style); invalidated when
  // Name, Size, Bold or Italic change. Used by Bitmap::DrawText()/TextSize().
  font::FontSystem& font_system();

 private:
  void ResetFontSystem();

  std::shared_ptr<font::FontSystem> font_system_;

  std::vector<std::string> name_;
  int size_ = 24;
  bool bold_ = false, italic_ = false, outline_ = true, shadow_ = false;
  RefPtr<Color> color_, out_color_;
};

}  // namespace lime
