#pragma once

#include "common.h"
#include "refptr.h"
#include "utility.h"

namespace rgssx {

class Font : public RefCounted<Font> {
 public:
  Font(std::vector<std::string> names = {}, int size = 24);
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
  raylib::Font& font() { return font_; }

 private:
  raylib::Font font_;

  std::vector<std::string> name_;
  int size_ = 24;
  bool bold_ = false, italic_ = false, outline_ = true, shadow_ = false;
  RefPtr<Color> color_, out_color_;
};

}  // namespace rgssx
