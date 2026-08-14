#pragma once

#include "src/raywarp.h"
#include "src/refptr.h"
#include "src/utility.h"

namespace lime {

class Palette : public RefCounted<Palette>, public Dispoable {
 public:
  Palette(raylib::Image data);
  Palette(int width, int height);
  Palette(std::string filename);
  ~Palette();

  /*-export.begin-*/
  RefPtr<Color> GetPixel(int x, int y);
  void SetPixel(int x, int y, RefPtr<Color> color);
  void SaveFile(std::string filename);
  /*-export.end-*/

 public:
  raylib::Image& image() { return image_; }

 private:
  void DisposeObject() override;

  raylib::Image image_;
};

}  // namespace lime
