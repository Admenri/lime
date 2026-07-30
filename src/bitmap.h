#pragma once

#include "common.h"
#include "refptr.h"
#include "utility.h"

namespace rgssx {

class Bitmap : public RefCounted<Bitmap>, public Dispoable {
 public:
  Bitmap(std::string filename);
  Bitmap(int width, int height);
  ~Bitmap();

  raylib::RenderTexture2D& render_texture() { return texture_; }

  int Width();
  int Height();
  RefPtr<Rect> GetRect();
  void Blt(int x,
           int y,
           RefPtr<Bitmap> src_bitmap,
           RefPtr<Rect> src_rect,
           int opacity = 255);
  void StretchBlt(RefPtr<Rect> dst_rect,
                  RefPtr<Bitmap> src_bitmap,
                  RefPtr<Rect> src_rect,
                  int opacity = 255);
  void FillRect(int x, int y, int width, int height, RefPtr<Color> color);
  void FillRect(RefPtr<Rect> rect, RefPtr<Color> color);
  void GradientFillRect(int x,
                        int y,
                        int width,
                        int height,
                        RefPtr<Color> color1,
                        RefPtr<Color> color2,
                        bool vertical = false);
  void GradientFillRect(RefPtr<Rect> rect,
                        RefPtr<Color> color1,
                        RefPtr<Color> color2,
                        bool vertical = false);
  void Clear();
  void ClearRect(int x, int y, int width, int height);
  void ClearRect(RefPtr<Rect> rect);
  RefPtr<Color> GetPixel(int x, int y);
  void SetPixel(int x, int y, RefPtr<Color> color);
  void HueChange(int hue);
  void Blur();
  void RadialBlur(int angle, int division);
  void DrawText(int x,
                int y,
                int width,
                int height,
                std::string str,
                int align = 0);
  void DrawText(RefPtr<Rect> rect, std::string str, int align = 0);
  RefPtr<Rect> TextSize(std::string str);

  void SaveFile(std::string filename);

 private:
  void DisposeObject() override;

  raylib::RenderTexture2D texture_ = {};
};

}  // namespace rgssx
