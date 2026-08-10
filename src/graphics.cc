#include "src/graphics.h"

#include "src/audio.h"

namespace lime {

Graphics::Graphics(int w,
                   int h,
                   std::string title,
                   bool vsync,
                   bool fullscreen) {
  // Flags
  uint32_t flags = raylib::FLAG_WINDOW_HIGHDPI;
  if (vsync)
    flags |= raylib::FLAG_VSYNC_HINT;
  if (fullscreen)
    flags |= raylib::FLAG_FULLSCREEN_MODE;
  raylib::SetConfigFlags(flags);

  // Init window
  raylib::InitWindow(w, h, title.c_str());
  raylib::SetTargetFPS(frame_rate_);
  raylib::SetExitKey(0);

  // Common shaders
  ShaderSet::Instance(new ShaderSet());

  // Screen buffer
  screen_buffer_ = raylib::LoadRenderTexture(w, h);

  // Default font premultiplied alpha
  auto font_atlas = raylib::GetFontDefault().texture;
  auto font_image = raylib::LoadImageFromTexture(font_atlas);
  raylib::ImageAlphaPremultiply(&font_image);
  raylib::UpdateTexture(font_atlas, font_image.data);

  // Avoid OpenGL flipping
  raylib::rlDisableBackfaceCulling();

  // Reset black screen
  raylib::BeginDrawing();
  raylib::Color clear_color = {0, 0, 0, 255};
  raylib::ClearBackground(clear_color);
  raylib::EndDrawing();
}

Graphics::~Graphics() {
  raylib::UnloadRenderTexture(screen_buffer_);
  ShaderSet::Instance(nullptr);
  raylib::CloseWindow();
}

void Graphics::Update() {
  if (!frozen_)
    RenderFrame(screen_buffer_);

  // Screen present
  raylib::BeginDrawing();
  raylib::rlSetBlendMode(raylib::BLEND_ALPHA_PREMULTIPLY);
  {
    raylib::ClearBackground({255, 0, 0, 255});

    raylib::Rectangle srcrec = {};
    srcrec.width = static_cast<float>(screen_buffer_.texture.width);
    srcrec.height = static_cast<float>(screen_buffer_.texture.height);

    raylib::Rectangle dstrec = {};
    dstrec.width = static_cast<float>(raylib::GetScreenWidth());
    dstrec.height = static_cast<float>(raylib::GetScreenHeight());

    raylib::DrawTexturePro(screen_buffer_.texture, srcrec, dstrec, {}, 0,
                           raylib::WHITE);
  }
  raylib::EndDrawing();

  UpdatePerFrame();

  if (raylib::WindowShouldClose())
    throw Exception(Exception::ExitError, "exit");
  if (raylib::IsKeyReleased(raylib::KEY_F12))
    throw Exception(Exception::ResetError, "reset");
}

void Graphics::Wait(int duration) {
  for (int i = 0; i < duration; ++i)
    Update();
}

void Graphics::FadeIn(int duration) {
  duration = std::max(duration, 1);
  int step = (255 - brightness_) / duration;
  for (int i = 0; i < duration + 1; ++i) {
    brightness_ += step;
    brightness_ = std::clamp<int>(brightness_, 0, 255);
    Update();
  }
}

void Graphics::FadeOut(int duration) {
  duration = std::max(duration, 1);
  int step = brightness_ / duration;
  for (int i = 0; i < duration + 1; ++i) {
    brightness_ -= step;
    brightness_ = std::clamp<int>(brightness_, 0, 255);
    Update();
  }
}

void Graphics::Freeze() {
  if (!frozen_) {
    RenderFrame(screen_buffer_);
    frozen_ = true;
  }
}

void Graphics::Transition(int duration, std::string filename, int vague) {
  RefPtr<Bitmap> mapping = nullptr;
  if (!filename.empty())
    mapping = MakeRefCounted<Bitmap>(filename);
  TransitionBitmap(duration, mapping, vague);
}

void Graphics::TransitionBitmap(int duration,
                                RefPtr<Bitmap> bitmap,
                                int vague) {
  if (frozen_) {
    brightness_ = 255;

    auto current_texture = raylib::LoadRenderTexture(Width(), Height());
    auto frozen_texture = raylib::LoadRenderTexture(Width(), Height());
    std::swap(screen_buffer_, frozen_texture);
    RenderFrame(current_texture);

    for (int i = 0; i < duration; ++i) {
      raylib::BeginTextureMode(screen_buffer_);
      raylib::rlDisableColorBlend();
      if (bitmap) {
        // Vague mapping
        auto& shader = ShaderSet::Instance()->mapping_trans;
        raylib::BeginShaderMode(shader.shader);
        {
          float progress_norm = i * (1.0f / duration),
                vague_norm = std::clamp<uint32_t>(vague, 1, 256) / 256.0f;

          raylib::SetShaderValueTexture(shader.shader, shader.u_frozen_image,
                                        frozen_texture.texture);
          raylib::SetShaderValueTexture(shader.shader, shader.u_mapping_image,
                                        bitmap->render_texture().texture);
          raylib::SetShaderValue(shader.shader, shader.u_progress,
                                 &progress_norm, raylib::SHADER_UNIFORM_FLOAT);
          raylib::SetShaderValue(shader.shader, shader.u_vague, &vague_norm,
                                 raylib::SHADER_UNIFORM_FLOAT);

          raylib::DrawTexture(current_texture.texture, 0, 0, {});
        }
        raylib::EndShaderMode();
      } else {
        // Alpha mapping
        auto& shader = ShaderSet::Instance()->alpha_trans;
        raylib::BeginShaderMode(shader.shader);
        {
          float progress_norm = i * (1.0f / duration);

          raylib::SetShaderValueTexture(shader.shader, shader.u_frozen_image,
                                        frozen_texture.texture);
          raylib::SetShaderValue(shader.shader, shader.u_progress,
                                 &progress_norm, raylib::SHADER_UNIFORM_FLOAT);

          raylib::DrawTexture(current_texture.texture, 0, 0, {});
        }
        raylib::EndShaderMode();
      }
      raylib::rlEnableColorBlend();
      raylib::EndTextureMode();

      Update();
    }

    raylib::UnloadRenderTexture(frozen_texture);
    raylib::UnloadRenderTexture(current_texture);
    frozen_ = false;
  }
}

RefPtr<Bitmap> Graphics::SnapToBitmap() {
  RefPtr<Bitmap> result = MakeRefCounted<Bitmap>(Width(), Height());
  RenderFrame(result->render_texture());
  return result;
}

void Graphics::FrameReset() {
  frame_count_ = 0;
}

int Graphics::Width() {
  return screen_buffer_.texture.width;
}

int Graphics::Height() {
  return screen_buffer_.texture.height;
}

void Graphics::ResizeScreen(int width, int height) {
  if (Width() != width || Height() != height) {
    raylib::UnloadRenderTexture(screen_buffer_);
    screen_buffer_ = raylib::LoadRenderTexture(width, height);

    // Set size and position
    int monitor = raylib::GetCurrentMonitor();
    int monitor_width = raylib::GetMonitorWidth(monitor);
    int monitor_height = raylib::GetMonitorHeight(monitor);
    raylib::SetWindowSize(width, height);
    raylib::SetWindowPosition((monitor_width - width) / 2,
                              (monitor_height - height) / 2);
  }
}

void Graphics::PlayMovie(std::string filename) {
  // TODO
}

void* Graphics::WindowHandle() {
  return raylib::GetWindowHandle();
}

ATTR_DEF(int, FrameRate, Graphics) {
  if (value.has_value()) {
    frame_rate_ = *value;
    raylib::SetTargetFPS(frame_rate_);
    return std::nullopt;
  } else {
    return frame_rate_;
  }
}

ATTR_DEF(int, FrameCount, Graphics) {
  if (value.has_value()) {
    frame_count_ = *value;
    return std::nullopt;
  } else {
    return frame_count_;
  }
}

ATTR_DEF(int, Brightness, Graphics) {
  if (value.has_value()) {
    brightness_ = std::clamp<int>(*value, 0, 255);
    return std::nullopt;
  } else {
    return brightness_;
  }
}

void Graphics::RenderFrame(raylib::RenderTexture2D target) {
  // Screen rendering
  raylib::BeginTextureMode(target);
  raylib::rlSetBlendMode(raylib::BLEND_ALPHA_PREMULTIPLY);
  {
    raylib::Color bgcolor = {0, 0, 0, 255};
    raylib::ClearBackground(bgcolor);

    DrawParam param = {};
    param.scissor = {};
    param.scissor.width = target.texture.width;
    param.scissor.height = target.texture.height;
    param.target = target;

    raylib::rlEnableScissorTest();
    raylib::rlScissor(0, 0, param.scissor.width, param.scissor.height);
    drawables_.DispatchDraw(param);
    raylib::rlDisableScissorTest();

    if (brightness_ < 255) {
      raylib::rlEnableColorBlend();
      raylib::rlSetBlendMode(raylib::BLEND_ALPHA_PREMULTIPLY);
      {
        raylib::Color brightness_norm = {};
        brightness_norm.a = 255 - brightness_;
        raylib::DrawRectangle(0, 0, target.texture.width, target.texture.height,
                              brightness_norm);
      }
    }
  }
  raylib::EndTextureMode();
}

void Graphics::UpdatePerFrame() {
  // Frame count increase
  frame_count_++;

  // Audio imexplicit update
  Audio::Instance()->Update();
}

}  // namespace lime
