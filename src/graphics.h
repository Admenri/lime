#pragma once

#include "src/bitmap.h"
#include "src/common.h"
#include "src/drawable.h"
#include "src/shader.h"

namespace lime {

class Graphics : public Singleton<Graphics> {
 public:
  Graphics(int w, int h, std::string title, bool vsync, bool fullscreen);
  ~Graphics();

  /*-export.begin-*/
  void Update();
  void Wait(int duration);
  void FadeIn(int duration);
  void FadeOut(int duration);
  void Freeze();
  void Transition(int duration = 10, std::string filename = {}, int vague = 40);
  void TransitionBitmap(int duration = 10,
                        RefPtr<Bitmap> bitmap = {},
                        int vague = 40);
  RefPtr<Bitmap> SnapToBitmap();
  void FrameReset();
  int Width();
  int Height();
  void ResizeScreen(int width, int height);
  void PlayMovie(std::string filename);

  void* WindowHandle();
  float Delta();

  ATTR(int, FrameRate);
  ATTR(int, FrameCount);
  ATTR(int, Brightness);

  ATTR(int, OX);
  ATTR(int, OY);
  /*-export.end-*/

 public:
  DrawableSet* drawable_set() { return &drawables_; }

  static void RenderFrame(DrawableSet* root,
                          raylib::RenderTexture2D target,
                          raylib::Color clear_color,
                          raylib::Vector2 origin,
                          int brightness = 255);

 private:
  void UpdatePerFrame();

  DrawableSet drawables_;
  raylib::RenderTexture2D screen_buffer_;

  bool frozen_ = false;
  int brightness_ = 255;
  int frame_count_ = 0;
  int frame_rate_ = 60;
  raylib::Vector2 origin_ = {};
};

}  // namespace lime
