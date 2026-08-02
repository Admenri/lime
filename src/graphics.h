#pragma once

#include "bitmap.h"
#include "common.h"
#include "drawable.h"
#include "shader.h"

namespace rgssx {

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

  ATTR(int, FrameRate);
  ATTR(int, FrameCount);
  ATTR(int, Brightness);
  /*-export.end-*/

 public:
  DrawableSet* drawable_set() { return &drawables_; }

 private:
  void RenderFrame(raylib::RenderTexture2D target);

  DrawableSet drawables_;
  raylib::RenderTexture2D screen_buffer_;

  bool frozen_ = false;
  int brightness_ = 255;
  int frame_count_ = 0;
  int frame_rate_ = 60;
};

inline raylib::BlendMode GetRaylibBlend(int type) {
  switch (type) {
    default:
    case 0:
      raylib::rlSetBlendFactorsSeparate(RL_ONE, RL_ONE_MINUS_SRC_ALPHA, RL_ONE,
                                        RL_ONE_MINUS_SRC_ALPHA, RL_FUNC_ADD,
                                        RL_FUNC_ADD);
      return raylib::BLEND_CUSTOM_SEPARATE;
    case 1:
      raylib::rlSetBlendFactorsSeparate(RL_ONE, RL_ONE, RL_ONE, RL_ONE,
                                        RL_FUNC_ADD, RL_FUNC_ADD);
      return raylib::BLEND_CUSTOM_SEPARATE;
    case 2:
      raylib::rlSetBlendFactorsSeparate(RL_ONE, RL_ONE, RL_ZERO, RL_ONE,
                                        RL_FUNC_REVERSE_SUBTRACT,
                                        RL_FUNC_REVERSE_SUBTRACT);
      return raylib::BLEND_CUSTOM_SEPARATE;
  }
}

}  // namespace rgssx
