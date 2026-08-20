// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Admenri Adev <admenri0504@gmail.com>.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the “Software”), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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
    RenderFrame(&drawables_, screen_buffer_, raylib::BLACK, origin_,
                brightness_);

  // Screen present
  raylib::BeginDrawing();
  raylib::rlEnableColorBlend();
  raylib::rlSetBlendMode(raylib::RL_BLEND_ALPHA_PREMULTIPLY);
  raylib::rlDisableScissorTest();
  {
    raylib::ClearBackground({148, 243, 244, 255});
    auto& texture = screen_buffer_.texture;

    raylib::Rectangle srcrec = {};
    srcrec.width = static_cast<float>(texture.width);
    srcrec.height = static_cast<float>(texture.height);

    raylib::Rectangle dstrec = {};
    dstrec.width = static_cast<float>(raylib::GetScreenWidth());
    dstrec.height = static_cast<float>(raylib::GetScreenHeight());

    raylib::DrawTexturePro(texture, srcrec, dstrec, {}, 0, raylib::WHITE);

    raylib::DrawFPS(10, 10);
  }
  raylib::EndDrawing();

  // Update frame
  UpdatePerFrame();

  // Update event
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
  for (int i = 0; i < duration; ++i) {
    brightness_ += step;
    brightness_ = std::clamp<int>(brightness_, 0, 255);

    Update();
  }
  brightness_ = 255;
}

void Graphics::FadeOut(int duration) {
  duration = std::max(duration, 1);
  int step = brightness_ / duration;
  for (int i = 0; i < duration; ++i) {
    brightness_ -= step;
    brightness_ = std::clamp<int>(brightness_, 0, 255);

    Update();
  }
  brightness_ = 0;
}

void Graphics::Freeze() {
  if (!frozen_) {
    RenderFrame(&drawables_, screen_buffer_, raylib::BLACK, origin_,
                brightness_);
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

    RenderFrame(&drawables_, current_texture, raylib::BLACK, origin_);

    std::swap(screen_buffer_, frozen_texture);

    for (int i = 0; i < duration; ++i) {
      raylib::BeginTextureMode(screen_buffer_);
      raylib::rlDisableScissorTest();
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
  RenderFrame(&drawables_, result->render_texture(), raylib::BLACK, origin_,
              brightness_);
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

float Graphics::Delta() {
  return raylib::GetFrameTime();
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

ATTR_DEF(int, OX, Graphics) {
  if (value.has_value()) {
    origin_.x = static_cast<float>(*value);
    return std::nullopt;
  } else {
    return static_cast<int>(origin_.x);
  }
}

ATTR_DEF(int, OY, Graphics) {
  if (value.has_value()) {
    origin_.y = static_cast<float>(*value);
    return std::nullopt;
  } else {
    return static_cast<int>(origin_.y);
  }
}

void Graphics::RenderFrame(DrawableSet* root,
                           raylib::RenderTexture2D target,
                           raylib::Color clear_color,
                           raylib::Vector2 origin,
                           int brightness) {
  int width = target.texture.width, height = target.texture.height;

  const auto last_scissor_enable = raylib::rlIsScissorEnabled();
  const auto last_scissor_rect = raylib::GetScissor();

  // Screen rendering
  raylib::BeginTextureMode(target);
  {
    raylib::ClearBackground(clear_color);

    DrawParam param = {};
    param.offset.x = -origin.x;
    param.offset.y = -origin.y;
    param.target = target;

    raylib::rlDrawRenderBatchActive();
    raylib::rlMatrixMode(RL_MODELVIEW);
    raylib::rlPushMatrix();
    {
      raylib::rlLoadIdentity();
      raylib::rlTranslatef(param.offset.x, param.offset.y, 0.0f);
      {
        raylib::rlEnableScissorTest();
        raylib::rlScissor(0, 0, width, height);
        root->DispatchDraw(param);
      }

      raylib::rlDrawRenderBatchActive();
      raylib::rlLoadIdentity();
      if (brightness < 255) {
        raylib::rlDisableScissorTest();
        raylib::rlEnableColorBlend();
        raylib::rlSetBlendMode(raylib::RL_BLEND_ALPHA_PREMULTIPLY);
        {
          raylib::Color brightness_norm = {};
          brightness_norm.a = 255 - brightness;
          raylib::DrawRectangle(0, 0, width, height, brightness_norm);
        }
      }
    }
    raylib::rlPopMatrix();
    raylib::rlDrawRenderBatchActive();
  }
  raylib::EndTextureMode();

  raylib::SetScissor(last_scissor_rect);
  raylib::SetScissorTest(last_scissor_enable);
}

void Graphics::UpdatePerFrame() {
  // Frame count increase
  frame_count_++;

  // Audio imexplicit update
  Audio::Instance()->Update();
}

}  // namespace lime
