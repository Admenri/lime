#include "src/viewport.h"

#include "src/graphics.h"

namespace rgssx {

ViewportChild::ViewportChild(RefPtr<Viewport> viewport, const ZValue& z)
    : Drawable(z), viewport_(viewport) {
  Drawable::SetParent(viewport_ ? viewport_->drawable_set()
                                : Graphics::Instance()->drawable_set());
}

ATTR_DEF(RefPtr<Viewport>, Viewport, ViewportChild) {
  if (value.has_value()) {
    viewport_ = *value;
    Drawable::SetParent(viewport_ ? viewport_->drawable_set() : nullptr);
    return std::nullopt;
  } else {
    return viewport_;
  }
}

// ---------------------------------------------------------------------------

Viewport::Viewport(RefPtr<Viewport> viewport,
                   int x,
                   int y,
                   int width,
                   int height)
    : ViewportChild(viewport, ZValue()),
      rect_(MakeRefCounted<Rect>(x, y, width, height)),
      color_(MakeRefCounted<Color>()),
      tone_(MakeRefCounted<Tone>()) {
  cache_ = raylib::LoadRenderTexture(width, height);
}

Viewport::Viewport(int x, int y, int width, int height)
    : Viewport(nullptr, x, y, width, height) {}

Viewport::Viewport(RefPtr<Viewport> viewport, RefPtr<Rect> rect)
    : Viewport(viewport, rect->x, rect->y, rect->width, rect->height) {}

Viewport::Viewport(RefPtr<Rect> rect)
    : Viewport(nullptr, rect->x, rect->y, rect->width, rect->height) {}

Viewport::Viewport()
    : Viewport(0,
               0,
               Graphics::Instance()->Width(),
               Graphics::Instance()->Height()) {}

Viewport::~Viewport() {
  Dispose();
}

void Viewport::Flash(RefPtr<Color> color, int duration) {
  flash_.color = color ? color->Normalize() : raylib::Vector4{};
  flash_.step =
      duration > 0 ? (flash_.color.w / static_cast<float>(duration)) : 0.0f;
}

void Viewport::Update() {
  flash_.color.w -= flash_.step;
  if (flash_.color.w <= 0) {
    flash_.color = {};
    flash_.step = 0.0f;
  }
}

void Viewport::Render(RefPtr<Bitmap> target) {
  if (target && !target->IsDisposed()) {
    // Preparing
    drawables_.DispatchPrepare();

    // Rendering
    raylib::BeginTextureMode(target->render_texture());
    raylib::BeginBlendMode(raylib::BLEND_ALPHA_PREMULTIPLY);
    raylib::rlEnableScissorTest();
    {
      raylib::Color bgcolor = {0, 0, 0, 255};
      raylib::ClearBackground(bgcolor);

      DrawParam param = {};
      param.scissor = {};
      param.scissor.width = target->Width();
      param.scissor.height = target->Height();
      param.target = target->render_texture();

      raylib::rlScissor(0, 0, param.scissor.width, param.scissor.height);
      drawables_.DispatchDraw(param);
    }
    raylib::rlDisableScissorTest();
    raylib::EndBlendMode();
    raylib::EndTextureMode();
  }
}

ATTR_DEF(RefPtr<Rect>, Rect, Viewport) {
  if (value.has_value()) {
    rect_->Set(*value);
    return std::nullopt;
  } else {
    return rect_;
  }
}

ATTR_DEF(int, OX, Viewport) {
  if (value.has_value()) {
    ox_ = *value;
    return std::nullopt;
  } else {
    return ox_;
  }
}

ATTR_DEF(int, OY, Viewport) {
  if (value.has_value()) {
    oy_ = *value;
    return std::nullopt;
  } else {
    return oy_;
  }
}

ATTR_DEF(float, Angle, Viewport) {
  if (value.has_value()) {
    angle_ = *value;
    return std::nullopt;
  } else {
    return angle_;
  }
}

ATTR_DEF(float, ZoomX, Viewport) {
  if (value.has_value()) {
    zoom_x_ = *value;
    return std::nullopt;
  } else {
    return zoom_x_;
  }
}

ATTR_DEF(float, ZoomY, Viewport) {
  if (value.has_value()) {
    zoom_y_ = *value;
    return std::nullopt;
  } else {
    return zoom_y_;
  }
}

ATTR_DEF(bool, Clip, Viewport) {
  if (value.has_value()) {
    clip_ = *value;
    return std::nullopt;
  } else {
    return clip_;
  }
}

ATTR_DEF(RefPtr<Color>, Color, Viewport) {
  if (value.has_value()) {
    color_->Set(*value);
    return std::nullopt;
  } else {
    return color_;
  }
}

ATTR_DEF(RefPtr<Tone>, Tone, Viewport) {
  if (value.has_value()) {
    tone_->Set(*value);
    return std::nullopt;
  } else {
    return tone_;
  }
}

void Viewport::DisposeObject() {
  Drawable::RemoveFromList();

  raylib::UnloadRenderTexture(cache_);
  cache_ = {};
}

void Viewport::Prepare() {
  // Dispatching to children
  drawables_.DispatchPrepare();
}

void Viewport::Draw(DrawParam param) {
  // Scissor region compute
  RectRegion viewport_region = {};
  viewport_region.x = param.ox + rect_->x;
  viewport_region.y = param.oy + rect_->y;
  viewport_region.width = rect_->width;
  viewport_region.height = rect_->height;

  // Set as viewport range
  raylib::rlDrawRenderBatchActive();
  RectRegion scissor_rect = param.scissor;
  if (clip_) {
    scissor_rect = RectRegion::MakeIntersect(param.scissor, viewport_region);
    raylib::rlScissor(scissor_rect.x, scissor_rect.y, scissor_rect.width,
                      scissor_rect.height);
  }

  {
    raylib::rlMatrixMode(RL_MODELVIEW);
    raylib::rlPushMatrix();
    {
      int offset_x = rect_->x - ox_, offset_y = rect_->y - oy_;
      raylib::rlTranslatef(static_cast<float>(offset_x),
                           static_cast<float>(offset_y), 0.0f);
      raylib::rlRotatef(angle_, 0.0f, 0.0f, 1.0f);
      raylib::rlScalef(zoom_x_, zoom_y_, 1.0f);

      // Dispatching drawcalls
      DrawParam draw_param = param;
      draw_param.ox = param.ox + rect_->x - ox_;
      draw_param.oy = param.oy + rect_->y - oy_;
      draw_param.scissor = scissor_rect;
      drawables_.DispatchDraw(draw_param);
    }
    raylib::rlPopMatrix();
  }

  // Restore scissor
  raylib::rlDrawRenderBatchActive();
  if (clip_) {
    raylib::rlScissor(param.scissor.x, param.scissor.y, param.scissor.width,
                      param.scissor.height);
  }

  // Viewport effect process
  if (clip_ && (tone_->HasEffect() || color_->alpha || flash_.color.w)) {
    if (cache_.texture.width != rect_->width ||
        cache_.texture.height != rect_->height) {
      raylib::UnloadRenderTexture(cache_);
      cache_ = raylib::LoadRenderTexture(rect_->width, rect_->height);
    }

    raylib::EndTextureMode();
    raylib::BeginTextureMode(cache_);
    {
      raylib::rlMatrixMode(RL_MODELVIEW);
      raylib::rlPushMatrix();
      raylib::rlLoadIdentity();
      {
        // Copy back buffer to current cache
        raylib::DrawTextureRec(param.target.texture, viewport_region.As(), {},
                               raylib::RAYWHITE);
      }
      raylib::rlPopMatrix();
    }
    raylib::EndTextureMode();
    raylib::BeginTextureMode(param.target);

    auto& shader = ShaderSet::Instance()->viewport;
    auto color_norm = color_->Normalize();
    auto tone_norm = tone_->Normalize();
    auto opacity_norm = 1.0f;

    if (flash_.color.w > color_norm.w)
      color_norm = flash_.color;

    raylib::BeginShaderMode(shader.shader);
    raylib::rlDrawRenderBatchActive();
    raylib::rlDisableColorBlend();
    {
      raylib::rlMatrixMode(RL_MODELVIEW);
      raylib::rlPushMatrix();
      raylib::rlLoadIdentity();
      {
        raylib::SetShaderValue(shader.shader, shader.u_color, &color_norm,
                               raylib::SHADER_UNIFORM_VEC4);
        raylib::SetShaderValue(shader.shader, shader.u_tone, &tone_norm,
                               raylib::SHADER_UNIFORM_VEC4);
        raylib::SetShaderValue(shader.shader, shader.u_opacity, &opacity_norm,
                               raylib::SHADER_UNIFORM_FLOAT);

        raylib::DrawTexture(cache_.texture, viewport_region.x,
                            viewport_region.y, raylib::RAYWHITE);
      }
      raylib::rlPopMatrix();
    }
    raylib::rlEnableColorBlend();
    raylib::rlDrawRenderBatchActive();
    raylib::EndShaderMode();
  }
}

}  // namespace rgssx
