#pragma once

#include "src/bitmap.h"
#include "src/common.h"
#include "src/drawable.h"
#include "src/refptr.h"
#include "src/utility.h"
#include "src/viewport.h"

namespace lime {

class Window : public RefCounted<Window>,
               public Dispoable,
               public ViewportChild {
 public:
  Window(int x, int y, int width, int height);
  Window();
  ~Window();

  /*-export.begin-*/
  void Update();
  void Move(int x, int y, int width, int height);
  bool Opened();
  bool Closed();

  ATTR(RefPtr<Bitmap>, WindowSkin);
  ATTR(RefPtr<Bitmap>, Contents);
  ATTR(RefPtr<Rect>, CursorRect);
  ATTR(bool, Active);
  ATTR(bool, ArrowsVisible);
  ATTR(bool, Pause);
  ATTR(int, X);
  ATTR(int, Y);
  ATTR(int, Width);
  ATTR(int, Height);
  ATTR(int, OX);
  ATTR(int, OY);
  ATTR(int, Padding);
  ATTR(int, PaddingBottom);
  ATTR(int, Opacity);
  ATTR(int, BackOpacity);
  ATTR(int, ContentsOpacity);
  ATTR(int, Openness);
  ATTR(RefPtr<Tone>, Tone);
  ATTR(int, Scale);
  /*-export.end-*/

 private:
  void DisposeObject() override;
  void Draw(DrawParam param) override;

  RefPtr<Bitmap> window_skin_;
  RefPtr<Bitmap> contents_;
  RefPtr<Rect> cursor_rect_;
  bool active_ = true, arrows_visible_ = true, pause_ = false;
  int x_ = 0, y_ = 0, width_ = 0, height_ = 0;
  int ox_ = 0, oy_ = 0;
  int padding_ = 12, padding_bottom_ = 12;
  int opacity_ = 255, back_opacity_ = 192, contents_opacity_ = 255;
  int openness_ = 255;
  RefPtr<Tone> tone_;
  int scale_ = 2;

  bool rgss3_style_ = true;
  int pause_index_ = 0;
  int cursor_index_ = 0;
};

}  // namespace lime
