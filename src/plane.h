#pragma once

#include "bitmap.h"
#include "common.h"
#include "drawable.h"
#include "refptr.h"
#include "viewport.h"

namespace rgssx {

class Plane : public RefCounted<Plane>, public Dispoable, public ViewportChild {
 public:
  Plane(RefPtr<Viewport> viewport = nullptr);
  ~Plane();

  /*-export.begin-*/
  ATTR(RefPtr<Bitmap>, Bitmap);
  ATTR(int, OX);
  ATTR(int, OY);
  ATTR(float, ZoomX);
  ATTR(float, ZoomY);
  ATTR(int, Opacity);
  ATTR(int, BlendType);
  ATTR(RefPtr<Color>, Color);
  ATTR(RefPtr<Tone>, Tone);
  /*-export.end-*/

 private:
  void DisposeObject() override;
  void Draw(DrawParam param) override;

  RefPtr<Bitmap> bitmap_;
  int ox_ = 0, oy_ = 0;
  float zoom_x_ = 1.0f, zoom_y_ = 1.0f;
  int opacity_ = 255, blend_type_ = 0;
  RefPtr<Color> color_;
  RefPtr<Tone> tone_;
};

}  // namespace rgssx
