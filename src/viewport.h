#pragma once

#include "src/bitmap.h"
#include "src/common.h"
#include "src/drawable.h"
#include "src/effect.h"
#include "src/refptr.h"
#include "src/utility.h"

namespace lime {

class Viewport;

class ViewportChild : public Drawable {
 public:
  ViewportChild(RefPtr<Viewport> viewport, const ZValue& z);

  /*-export.begin-*/
  virtual ATTR(RefPtr<Viewport>, Viewport);
  /*-export.end-*/

 private:
  RefPtr<Viewport> viewport_;
};

class Viewport : public RefCounted<Viewport>,
                 public Dispoable,
                 public ViewportChild {
 public:
  Viewport(RefPtr<Viewport> viewport, int x, int y, int width, int height);
  Viewport(int x, int y, int width, int height);
  Viewport(RefPtr<Viewport> viewport, RefPtr<Rect> rect);
  Viewport(RefPtr<Rect> rect);
  Viewport();
  ~Viewport();

  /*-export.begin-*/
  void Flash(RefPtr<Color> color, int duration);
  void Update();

  void Render(RefPtr<Bitmap> target);

  ATTR(RefPtr<Rect>, Rect);
  ATTR(int, OX);
  ATTR(int, OY);
  ATTR(float, Angle);
  ATTR(float, ZoomX);
  ATTR(float, ZoomY);
  ATTR(bool, Clip);
  ATTR(RefPtr<Color>, Color);
  ATTR(RefPtr<Tone>, Tone);
  ATTR(RefPtr<Effect>, Effect);
  /*-export.end-*/

 public:
  DrawableSet* drawable_set() { return &drawables_; }

 private:
  void DisposeObject() override;
  void Draw(DrawParam param) override;
  void UpdateCacheTexture();

  DrawableSet drawables_;
  raylib::RenderTexture2D cache_ = {};

  RefPtr<Rect> rect_;
  int ox_ = 0, oy_ = 0;
  float angle_ = 0.0f;
  float zoom_x_ = 1.0f, zoom_y_ = 1.0f;
  bool clip_ = true;
  RefPtr<Color> color_;
  RefPtr<Tone> tone_;
  RefPtr<Effect> effect_;

  struct {
    raylib::Vector4 color = {};
    float step = 0.0f;
  } flash_;
};

}  // namespace lime
