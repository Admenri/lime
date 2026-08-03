#include "src/font.h"

namespace rgssx {

Font::Font(std::vector<std::string> names, int size)
    : name_(names),
      size_(size),
      color_(MakeRefCounted<Color>()),
      out_color_(MakeRefCounted<Color>()) {}

Font::~Font() {}

// static
bool Font::Exist(std::string name) {
  return false;
}

ATTR_DEF(std::vector<std::string>, Name, Font) {
  if (value.has_value()) {
    name_ = *value;
    return std::nullopt;
  } else {
    return name_;
  }
}

ATTR_DEF(int, Size, Font) {
  if (value.has_value()) {
    size_ = *value;
    return std::nullopt;
  } else {
    return size_;
  }
}

ATTR_DEF(bool, Bold, Font) {
  if (value.has_value()) {
    bold_ = *value;
    return std::nullopt;
  } else {
    return bold_;
  }
}

ATTR_DEF(bool, Italic, Font) {
  if (value.has_value()) {
    italic_ = *value;
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
  static std::vector<std::string> default_names;
  if (value.has_value()) {
    default_names = *value;
    return std::nullopt;
  } else {
    return default_names;
  }
}

ATTR_DEF(int, DefaultSize, Font) {
  static int default_size;
  if (value.has_value()) {
    default_size = *value;
    return std::nullopt;
  } else {
    return default_size;
  }
}

ATTR_DEF(bool, DefaultBold, Font) {
  static bool default_bold;
  if (value.has_value()) {
    default_bold = *value;
    return std::nullopt;
  } else {
    return default_bold;
  }
}

ATTR_DEF(bool, DefaultItalic, Font) {
  static bool default_italic;
  if (value.has_value()) {
    default_italic = *value;
    return std::nullopt;
  } else {
    return default_italic;
  }
}

ATTR_DEF(bool, DefaultOutline, Font) {
  static bool default_outline;
  if (value.has_value()) {
    default_outline = *value;
    return std::nullopt;
  } else {
    return default_outline;
  }
}

ATTR_DEF(bool, DefaultShadow, Font) {
  static bool default_shadow;
  if (value.has_value()) {
    default_shadow = *value;
    return std::nullopt;
  } else {
    return default_shadow;
  }
}

ATTR_DEF(RefPtr<Color>, DefaultColor, Font) {
  static RefPtr<Color> default_color;
  if (value.has_value()) {
    default_color = *value;
    return std::nullopt;
  } else {
    return default_color;
  }
}

ATTR_DEF(RefPtr<Color>, DefaultOutColor, Font) {
  static RefPtr<Color> default_out_color;
  if (value.has_value()) {
    default_out_color = *value;
    return std::nullopt;
  } else {
    return default_out_color;
  }
}

}  // namespace rgssx
