// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Admenri Adev <admenri0504@gmail.com>.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "src/bitmap.h"

#include "src/filesystem.h"
#include "src/font/font_system.h"
#include "src/shader.h"

namespace lime {

// RAII wrapper that opens the render target for shape drawing and enables
// standard alpha blending (raylib's default for shapes). Declared in the
// lime namespace (not anonymous) so it matches the `friend class
// ShapeDrawScope;` declaration in Bitmap, which grants it access to the
// private texture_ / shape_bitmap_ members.
class CommonDrawScope {
 public:
  explicit CommonDrawScope(Bitmap* context, bool blend) {
    raylib::BeginTextureMode(context->texture_);
    if (blend) {
      raylib::rlEnableColorBlend();
      raylib::rlSetBlendMode(raylib::RL_BLEND_ALPHA_PREMULTIPLY);
    } else {
      raylib::rlDisableColorBlend();
    }
  }

  ~CommonDrawScope() { raylib::EndTextureMode(); }
};

class ShapeDrawScope {
 public:
  explicit ShapeDrawScope(Bitmap* context) {
    raylib::BeginTextureMode(context->texture_);
    raylib::rlEnableColorBlend();
    raylib::rlSetBlendMode(raylib::RL_BLEND_ALPHA_PREMULTIPLY);
    if (auto& texture = context->shape_bitmap_; texture) {
      raylib::Rectangle rec = {};
      rec.width = texture->GetWidth();
      rec.height = texture->GetHeight();
      raylib::SetShapesTexture(texture->texture_.texture, rec);
    }
  }

  ~ShapeDrawScope() {
    raylib::SetShapesTexture({}, {});
    raylib::EndTextureMode();
  }
};

namespace {

// Converts an array of wrapped Vector2 into a flat raylib Vector2 buffer so
// it can be passed to the raylib shape functions.
std::vector<raylib::Vector2> ToRaylibPoints(
    const std::vector<RefPtr<Vector2>>& points) {
  std::vector<raylib::Vector2> result;
  result.reserve(points.size());
  for (const auto& point : points) {
    if (!point)
      throw Exception(Exception::RGSSError, "invalid vector.");
    result.push_back(point->As());
  }
  return result;
}

// Converts an RGBA color to premultiplied alpha: the RGB channels are scaled
// by the normalized alpha value so the color composites correctly in the
// premultiplied blend mode used by the render texture (see StretchBlt).
raylib::Color PremultiplyColor(RefPtr<Color> color) {
  auto result = color->As();
  const float factor = result.a / 255.0f;
  result.r = static_cast<uint8_t>(result.r * factor);
  result.g = static_cast<uint8_t>(result.g * factor);
  result.b = static_cast<uint8_t>(result.b * factor);
  return result;
}

}  // namespace

#define CHECK_VALUE(x) \
  if (!x)              \
    throw Exception(Exception::RGSSError, "invalid value: " #x);

#define CHECK_OBJ(x)        \
  if (!Dispoable::Check(x)) \
    throw Exception(Exception::RGSSError, "invalid object: " #x);

Bitmap::Bitmap(std::string filename) : font_(MakeRefCounted<Font>()) {
  raylib::Image image = {};

  // RGSS style loading: the extension may be omitted and the virtual file
  // system will resolve the actual file (e.g. "Iconset" -> "Iconset.png").
  IOService::Instance()->OpenRead(
      filename, [&](std::unique_ptr<IOStream> stream, const std::string& ext) {
        auto data = stream->ReadAll();

        // raylib needs a file type hint (e.g. ".png")
        std::string file_type = "." + ext;
        image = raylib::LoadImageFromMemory(
            file_type.c_str(), (uint8_t*)data.data(), (int)data.size());
        if (!image.data)
          return false;

        return true;  // matched, stop enumeration
      });

  if (!image.data)
    throw Exception(Exception::RGSSError, "failed to load image: {}", filename);

  raylib::ImageFormat(&image, raylib::PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
  raylib::ImageAlphaPremultiply(&image);

  texture_ = raylib::LoadRenderTexture(image.width, image.height);
  raylib::UpdateTexture(texture_.texture, image.data);

  raylib::UnloadImage(image);
}

Bitmap::Bitmap(int width, int height) : font_(MakeRefCounted<Font>()) {
  texture_ = raylib::LoadRenderTexture(width, height);
  Clear();
}

Bitmap::Bitmap(RefPtr<Bitmap> other)
    : font_(MakeRefCounted<Font>(other->font_)) {
  texture_ = raylib::LoadRenderTexture(other->GetWidth(), other->GetHeight());

  CommonDrawScope scope(this, false);
  raylib::DrawTexture(other->handle().texture, 0, 0, raylib::WHITE);
}

Bitmap::~Bitmap() {
  Dispose();
}

int Bitmap::GetWidth() {
  return texture_.texture.width;
}

int Bitmap::GetHeight() {
  return texture_.texture.height;
}

RefPtr<Rect> Bitmap::GetRect() {
  return MakeRefCounted<Rect>(0, 0, GetWidth(), GetHeight());
}

void Bitmap::Blt(int x,
                 int y,
                 RefPtr<Bitmap> src_bitmap,
                 RefPtr<Rect> src_rect,
                 int opacity) {
  Dispoable::Guard();

  CHECK_VALUE(src_rect);

  StretchBlt(MakeRefCounted<Rect>(x, y, src_rect->width, src_rect->height),
             src_bitmap, src_rect, opacity);
}

void Bitmap::StretchBlt(RefPtr<Rect> dst_rect,
                        RefPtr<Bitmap> src_bitmap,
                        RefPtr<Rect> src_rect,
                        int opacity) {
  Dispoable::Guard();

  CHECK_VALUE(dst_rect);
  CHECK_OBJ(src_bitmap);
  CHECK_VALUE(src_rect);

  CommonDrawScope scope(this, true);
  raylib::DrawTexturePro(src_bitmap->handle().texture, src_rect->As(),
                         dst_rect->As(), {}, 0,
                         raylib::MakeColor(std::clamp<int>(opacity, 0, 255)));
}

void Bitmap::FillRect(int x,
                      int y,
                      int width,
                      int height,
                      RefPtr<Color> color) {
  Dispoable::Guard();

  CHECK_VALUE(color);

  CommonDrawScope scope(this, false);
  raylib::DrawRectangle(x, y, width, height, color->As());
}

void Bitmap::FillRect(RefPtr<Rect> rect, RefPtr<Color> color) {
  Dispoable::Guard();

  CHECK_VALUE(rect);

  FillRect(rect->x, rect->y, rect->width, rect->height, color);
}

void Bitmap::GradientFillRect(int x,
                              int y,
                              int width,
                              int height,
                              RefPtr<Color> color1,
                              RefPtr<Color> color2,
                              bool vertical) {
  Dispoable::Guard();

  CHECK_VALUE(color1);
  CHECK_VALUE(color2);

  CommonDrawScope scope(this, false);
  if (vertical) {
    raylib::DrawRectangleGradientV(x, y, width, height, color1->As(),
                                   color2->As());
  } else {
    raylib::DrawRectangleGradientH(x, y, width, height, color1->As(),
                                   color2->As());
  }
}

void Bitmap::GradientFillRect(RefPtr<Rect> rect,
                              RefPtr<Color> color1,
                              RefPtr<Color> color2,
                              bool vertical) {
  Dispoable::Guard();

  CHECK_VALUE(rect);

  GradientFillRect(rect->x, rect->y, rect->width, rect->height, color1, color2,
                   vertical);
}

void Bitmap::Clear() {
  Dispoable::Guard();

  CommonDrawScope scope(this, false);
  raylib::ClearBackground({});
}

void Bitmap::ClearRect(int x, int y, int width, int height) {
  Dispoable::Guard();

  CommonDrawScope scope(this, false);
  raylib::DrawRectangle(x, y, width, height, {});
}

void Bitmap::ClearRect(RefPtr<Rect> rect) {
  Dispoable::Guard();

  CHECK_VALUE(rect);

  ClearRect(rect->x, rect->y, rect->width, rect->height);
}

RefPtr<Color> Bitmap::GetPixel(int x, int y) {
  Dispoable::Guard();

  auto image = raylib::LoadImageFromTexture(texture_.texture);
  auto color = raylib::GetImageColor(image, x, y);
  raylib::UnloadImage(image);

  float alpha = (float)color.a / 255.0f;
  color.r = (unsigned char)((float)color.r / alpha);
  color.g = (unsigned char)((float)color.g / alpha);
  color.b = (unsigned char)((float)color.b / alpha);
  return MakeRefCounted<Color>(color);
}

void Bitmap::SetPixel(int x, int y, RefPtr<Color> color) {
  Dispoable::Guard();

  CHECK_VALUE(color);

  CommonDrawScope scope(this, false);
  raylib::DrawPixel(x, y, PremultiplyColor(color));
}

void Bitmap::HueChange(int hue) {
  Dispoable::Guard();
  // TODO
}

void Bitmap::Blur() {
  Dispoable::Guard();
  // TODO
}

void Bitmap::RadialBlur(int angle, int division) {
  Dispoable::Guard();
  // TODO
}

void Bitmap::DrawText(int x,
                      int y,
                      int width,
                      int height,
                      std::string str,
                      int align) {
  Dispoable::Guard();
  // TODO

  if (!font_ || str.empty())
    return;

  auto& fs = font_->font_system();
  auto measured = fs.MeasureText(str.c_str());

  // Horizontal position by alignment (0 = left, 1 = center, 2 = right).
  float tx = static_cast<float>(x);
  if (align == 1)
    tx = x + (width - measured.x) / 2.0f;
  else if (align == 2)
    tx = x + (width - measured.x);

  // RGSS centers text vertically inside the box.
  float ty = y + (height - measured.y) / 2.0f;

  auto text_color = raylib::WHITE;
  if (auto c = font_->Attr_Color(); c.has_value() && *c)
    text_color = (*c)->As();
  auto out_color = raylib::BLACK;
  if (auto c = font_->Attr_OutColor(); c.has_value() && *c)
    out_color = (*c)->As();
  const bool shadow = font_->Attr_Shadow().value_or(false);
  const bool outline = font_->Attr_Outline().value_or(false);

  raylib::BeginTextureMode(texture_);
  {
    raylib::rlEnableColorBlend();

    // Drop shadow: a dark copy offset by 2px.
    if (shadow)
      fs.DrawText(str.c_str(), {tx + 2.0f, ty + 2.0f}, {0, 0, 0, 128});

    // Outline: 1px stroke in the outline color (8 directions).
    if (outline) {
      static const raylib::Vector2 kOutlineOffsets[] = {
          {-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {1, -1}, {-1, 1}, {1, 1}};
      for (const auto& off : kOutlineOffsets)
        fs.DrawText(str.c_str(), {tx + off.x, ty + off.y}, out_color);
    }

    // Main text.
    fs.DrawText(str.c_str(), {tx, ty}, text_color);
  }
  raylib::EndTextureMode();
}

void Bitmap::DrawText(RefPtr<Rect> rect, std::string str, int align) {
  Dispoable::Guard();

  CHECK_VALUE(rect);

  DrawText(rect->x, rect->y, rect->width, rect->height, str, align);
}

RefPtr<Rect> Bitmap::TextSize(std::string str) {
  Dispoable::Guard();
  // TODO

  if (!font_)
    return MakeRefCounted<Rect>();

  auto& fs = font_->font_system();
  auto size = fs.MeasureText(str.c_str());
  return MakeRefCounted<Rect>(0, 0, static_cast<int>(size.x),
                              static_cast<int>(size.y));
}

void Bitmap::MaskBlt(RefPtr<Rect> dst_rect,
                     RefPtr<Bitmap> src_bitmap,
                     RefPtr<Rect> src_rect,
                     RefPtr<Bitmap> mask) {
  Dispoable::Guard();

  CHECK_VALUE(dst_rect);
  CHECK_OBJ(src_bitmap);
  CHECK_VALUE(src_rect);
  CHECK_OBJ(mask);

  CommonDrawScope scope(this, false);

  auto& shader = ShaderSet::Instance()->bitmap_mask;
  raylib::BeginShaderMode(shader.shader);
  raylib::SetShaderValueTexture(shader.shader, shader.u_mask,
                                mask->handle().texture);
  raylib::DrawTexturePro(src_bitmap->handle().texture, src_rect->As(),
                         dst_rect->As(), {}, 0, {});
  raylib::EndShaderMode();
}

RefPtr<Palette> Bitmap::ToPalette() {
  Dispoable::Guard();
  return MakeRefCounted<Palette>(
      raylib::LoadImageFromTexture(texture_.texture));
}

void Bitmap::UpdateWithPalette(RefPtr<Palette> palette) {
  Dispoable::Guard();

  CHECK_VALUE(palette);

  raylib::UpdateTexture(texture_.texture, palette->image().data);
}

void Bitmap::SetFilter(int value) {
  raylib::rlTextureParameters(texture_.texture.id, RL_TEXTURE_MIN_FILTER,
                              value);
  raylib::rlTextureParameters(texture_.texture.id, RL_TEXTURE_MAG_FILTER,
                              value);
}

void Bitmap::SetWrap(int value) {
  raylib::rlTextureParameters(texture_.texture.id, RL_TEXTURE_WRAP_S, value);
  raylib::rlTextureParameters(texture_.texture.id, RL_TEXTURE_WRAP_T, value);
}

// ---------------------------------------------------------------------------
// Basic shapes drawing functions (raylib passthrough)
// ---------------------------------------------------------------------------

void Bitmap::DrawPixel(int x, int y, RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(color);

  ShapeDrawScope scope(this);
  raylib::DrawPixel(x, y, PremultiplyColor(color));
}

void Bitmap::DrawLine(int x1, int y1, int x2, int y2, RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(color);

  ShapeDrawScope scope(this);
  raylib::DrawLine(x1, y1, x2, y2, PremultiplyColor(color));
}

void Bitmap::DrawLine(RefPtr<Vector2> start_pos,
                      RefPtr<Vector2> end_pos,
                      RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(start_pos);
  CHECK_VALUE(end_pos);
  CHECK_VALUE(color);

  ShapeDrawScope scope(this);
  raylib::DrawLineV(start_pos->As(), end_pos->As(), PremultiplyColor(color));
}

void Bitmap::DrawLineEx(RefPtr<Vector2> start_pos,
                        RefPtr<Vector2> end_pos,
                        float thick,
                        RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(start_pos);
  CHECK_VALUE(end_pos);
  CHECK_VALUE(color);

  ShapeDrawScope scope(this);
  raylib::DrawLineEx(start_pos->As(), end_pos->As(), thick,
                     PremultiplyColor(color));
}

void Bitmap::DrawLineStrip(std::vector<RefPtr<Vector2>> points,
                           RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(color);

  auto rpoints = ToRaylibPoints(points);
  ShapeDrawScope scope(this);
  raylib::DrawLineStrip(rpoints.data(), (int)rpoints.size(),
                        PremultiplyColor(color));
}

void Bitmap::DrawLineBezier(RefPtr<Vector2> start_pos,
                            RefPtr<Vector2> end_pos,
                            float thick,
                            RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(start_pos);
  CHECK_VALUE(end_pos);
  CHECK_VALUE(color);

  ShapeDrawScope scope(this);
  raylib::DrawLineBezier(start_pos->As(), end_pos->As(), thick,
                         PremultiplyColor(color));
}

void Bitmap::DrawLineDashed(RefPtr<Vector2> start_pos,
                            RefPtr<Vector2> end_pos,
                            int dash_size,
                            int space_size,
                            RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(start_pos);
  CHECK_VALUE(end_pos);
  CHECK_VALUE(color);

  ShapeDrawScope scope(this);
  raylib::DrawLineDashed(start_pos->As(), end_pos->As(), dash_size, space_size,
                         PremultiplyColor(color));
}

void Bitmap::DrawTriangle(RefPtr<Vector2> v1,
                          RefPtr<Vector2> v2,
                          RefPtr<Vector2> v3,
                          RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(v1);
  CHECK_VALUE(v2);
  CHECK_VALUE(v3);
  CHECK_VALUE(color);

  ShapeDrawScope scope(this);
  raylib::DrawTriangle(v1->As(), v2->As(), v3->As(), PremultiplyColor(color));
}

void Bitmap::DrawTriangleGradient(RefPtr<Vector2> v1,
                                  RefPtr<Vector2> v2,
                                  RefPtr<Vector2> v3,
                                  RefPtr<Color> c1,
                                  RefPtr<Color> c2,
                                  RefPtr<Color> c3) {
  Dispoable::Guard();
  CHECK_VALUE(v1);
  CHECK_VALUE(v2);
  CHECK_VALUE(v3);
  CHECK_VALUE(c1);
  CHECK_VALUE(c2);
  CHECK_VALUE(c3);

  ShapeDrawScope scope(this);
  raylib::DrawTriangleGradient(v1->As(), v2->As(), v3->As(),
                               PremultiplyColor(c1), PremultiplyColor(c2),
                               PremultiplyColor(c3));
}

void Bitmap::DrawTriangleLines(RefPtr<Vector2> v1,
                               RefPtr<Vector2> v2,
                               RefPtr<Vector2> v3,
                               RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(v1);
  CHECK_VALUE(v2);
  CHECK_VALUE(v3);
  CHECK_VALUE(color);

  ShapeDrawScope scope(this);
  raylib::DrawTriangleLines(v1->As(), v2->As(), v3->As(),
                            PremultiplyColor(color));
}

void Bitmap::DrawTriangleFan(std::vector<RefPtr<Vector2>> points,
                             RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(color);

  auto rpoints = ToRaylibPoints(points);
  ShapeDrawScope scope(this);
  raylib::DrawTriangleFan(rpoints.data(), (int)rpoints.size(),
                          PremultiplyColor(color));
}

void Bitmap::DrawTriangleStrip(std::vector<RefPtr<Vector2>> points,
                               RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(color);

  auto rpoints = ToRaylibPoints(points);
  ShapeDrawScope scope(this);
  raylib::DrawTriangleStrip(rpoints.data(), (int)rpoints.size(),
                            PremultiplyColor(color));
}

void Bitmap::DrawRectangle(int x,
                           int y,
                           int width,
                           int height,
                           RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(color);

  ShapeDrawScope scope(this);
  raylib::DrawRectangle(x, y, width, height, PremultiplyColor(color));
}

void Bitmap::DrawRectangle(RefPtr<Vector2> position,
                           RefPtr<Vector2> size,
                           RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(position);
  CHECK_VALUE(size);
  CHECK_VALUE(color);

  ShapeDrawScope scope(this);
  raylib::DrawRectangleV(position->As(), size->As(), PremultiplyColor(color));
}

void Bitmap::DrawRectangle(RefPtr<Rect> rect, RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(rect);
  CHECK_VALUE(color);

  ShapeDrawScope scope(this);
  raylib::DrawRectangleRec(rect->As(), PremultiplyColor(color));
}

void Bitmap::DrawRectanglePro(RefPtr<Rect> rect,
                              RefPtr<Vector2> origin,
                              float rotation,
                              RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(rect);
  CHECK_VALUE(origin);
  CHECK_VALUE(color);

  ShapeDrawScope scope(this);
  raylib::DrawRectanglePro(rect->As(), origin->As(), rotation,
                           PremultiplyColor(color));
}

void Bitmap::DrawRectangleGradientV(int x,
                                    int y,
                                    int width,
                                    int height,
                                    RefPtr<Color> top,
                                    RefPtr<Color> bottom) {
  Dispoable::Guard();
  CHECK_VALUE(top);
  CHECK_VALUE(bottom);

  ShapeDrawScope scope(this);
  raylib::DrawRectangleGradientV(x, y, width, height, PremultiplyColor(top),
                                 PremultiplyColor(bottom));
}

void Bitmap::DrawRectangleGradientH(int x,
                                    int y,
                                    int width,
                                    int height,
                                    RefPtr<Color> left,
                                    RefPtr<Color> right) {
  Dispoable::Guard();
  CHECK_VALUE(left);
  CHECK_VALUE(right);

  ShapeDrawScope scope(this);
  raylib::DrawRectangleGradientH(x, y, width, height, PremultiplyColor(left),
                                 PremultiplyColor(right));
}

void Bitmap::DrawRectangleGradientEx(RefPtr<Rect> rect,
                                     RefPtr<Color> col1,
                                     RefPtr<Color> col2,
                                     RefPtr<Color> col3,
                                     RefPtr<Color> col4) {
  Dispoable::Guard();
  CHECK_VALUE(rect);
  CHECK_VALUE(col1);
  CHECK_VALUE(col2);
  CHECK_VALUE(col3);
  CHECK_VALUE(col4);

  ShapeDrawScope scope(this);
  raylib::DrawRectangleGradientEx(
      rect->As(), PremultiplyColor(col1), PremultiplyColor(col2),
      PremultiplyColor(col3), PremultiplyColor(col4));
}

void Bitmap::DrawRectangleLines(int x,
                                int y,
                                int width,
                                int height,
                                RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(color);

  ShapeDrawScope scope(this);
  raylib::DrawRectangleLines(x, y, width, height, PremultiplyColor(color));
}

void Bitmap::DrawRectangleLinesEx(RefPtr<Rect> rect,
                                  float thick,
                                  RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(rect);
  CHECK_VALUE(color);

  ShapeDrawScope scope(this);
  raylib::DrawRectangleLinesEx(rect->As(), thick, PremultiplyColor(color));
}

void Bitmap::DrawRectangleRounded(RefPtr<Rect> rect,
                                  float roundness,
                                  int segments,
                                  RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(rect);
  CHECK_VALUE(color);

  ShapeDrawScope scope(this);
  raylib::DrawRectangleRounded(rect->As(), roundness, segments,
                               PremultiplyColor(color));
}

void Bitmap::DrawRectangleRoundedLines(RefPtr<Rect> rect,
                                       float roundness,
                                       int segments,
                                       RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(rect);
  CHECK_VALUE(color);

  ShapeDrawScope scope(this);
  raylib::DrawRectangleRoundedLines(rect->As(), roundness, segments,
                                    PremultiplyColor(color));
}

void Bitmap::DrawRectangleRoundedLinesEx(RefPtr<Rect> rect,
                                         float roundness,
                                         int segments,
                                         float thick,
                                         RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(rect);
  CHECK_VALUE(color);

  ShapeDrawScope scope(this);
  raylib::DrawRectangleRoundedLinesEx(rect->As(), roundness, segments, thick,
                                      PremultiplyColor(color));
}

void Bitmap::DrawPoly(RefPtr<Vector2> center,
                      int sides,
                      float radius,
                      float rotation,
                      RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(center);
  CHECK_VALUE(color);

  ShapeDrawScope scope(this);
  raylib::DrawPoly(center->As(), sides, radius, rotation,
                   PremultiplyColor(color));
}

void Bitmap::DrawPolyLines(RefPtr<Vector2> center,
                           int sides,
                           float radius,
                           float rotation,
                           RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(center);
  CHECK_VALUE(color);

  ShapeDrawScope scope(this);
  raylib::DrawPolyLines(center->As(), sides, radius, rotation,
                        PremultiplyColor(color));
}

void Bitmap::DrawPolyLinesEx(RefPtr<Vector2> center,
                             int sides,
                             float radius,
                             float rotation,
                             float thick,
                             RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(center);
  CHECK_VALUE(color);

  ShapeDrawScope scope(this);
  raylib::DrawPolyLinesEx(center->As(), sides, radius, rotation, thick,
                          PremultiplyColor(color));
}

void Bitmap::DrawCircle(int center_x,
                        int center_y,
                        float radius,
                        RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(color);

  ShapeDrawScope scope(this);
  raylib::DrawCircle(center_x, center_y, radius, PremultiplyColor(color));
}

void Bitmap::DrawCircle(RefPtr<Vector2> center,
                        float radius,
                        RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(center);
  CHECK_VALUE(color);

  ShapeDrawScope scope(this);
  raylib::DrawCircleV(center->As(), radius, PremultiplyColor(color));
}

void Bitmap::DrawCircleGradient(RefPtr<Vector2> center,
                                float radius,
                                RefPtr<Color> inner,
                                RefPtr<Color> outer) {
  Dispoable::Guard();
  CHECK_VALUE(center);
  CHECK_VALUE(inner);
  CHECK_VALUE(outer);

  ShapeDrawScope scope(this);
  raylib::DrawCircleGradient(center->As(), radius, PremultiplyColor(inner),
                             PremultiplyColor(outer));
}

void Bitmap::DrawCircleSector(RefPtr<Vector2> center,
                              float radius,
                              float start_angle,
                              float end_angle,
                              int segments,
                              RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(center);
  CHECK_VALUE(color);

  ShapeDrawScope scope(this);
  raylib::DrawCircleSector(center->As(), radius, start_angle, end_angle,
                           segments, PremultiplyColor(color));
}

void Bitmap::DrawCircleSectorLines(RefPtr<Vector2> center,
                                   float radius,
                                   float start_angle,
                                   float end_angle,
                                   int segments,
                                   RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(center);
  CHECK_VALUE(color);

  ShapeDrawScope scope(this);
  raylib::DrawCircleSectorLines(center->As(), radius, start_angle, end_angle,
                                segments, PremultiplyColor(color));
}

void Bitmap::DrawCircleLines(int center_x,
                             int center_y,
                             float radius,
                             RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(color);

  ShapeDrawScope scope(this);
  raylib::DrawCircleLines(center_x, center_y, radius, PremultiplyColor(color));
}

void Bitmap::DrawCircleLines(RefPtr<Vector2> center,
                             float radius,
                             RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(center);
  CHECK_VALUE(color);

  ShapeDrawScope scope(this);
  raylib::DrawCircleLinesV(center->As(), radius, PremultiplyColor(color));
}

void Bitmap::DrawCircleLinesEx(RefPtr<Vector2> center,
                               float radius,
                               float thick,
                               RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(center);
  CHECK_VALUE(color);

  ShapeDrawScope scope(this);
  raylib::DrawCircleLinesEx(center->As(), radius, thick,
                            PremultiplyColor(color));
}

void Bitmap::DrawEllipse(int center_x,
                         int center_y,
                         float radius_h,
                         float radius_v,
                         RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(color);

  ShapeDrawScope scope(this);
  raylib::DrawEllipse(center_x, center_y, radius_h, radius_v,
                      PremultiplyColor(color));
}

void Bitmap::DrawEllipse(RefPtr<Vector2> center,
                         float radius_h,
                         float radius_v,
                         RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(center);
  CHECK_VALUE(color);

  ShapeDrawScope scope(this);
  raylib::DrawEllipseV(center->As(), radius_h, radius_v,
                       PremultiplyColor(color));
}

void Bitmap::DrawEllipseLines(int center_x,
                              int center_y,
                              float radius_h,
                              float radius_v,
                              RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(color);

  ShapeDrawScope scope(this);
  raylib::DrawEllipseLines(center_x, center_y, radius_h, radius_v,
                           PremultiplyColor(color));
}

void Bitmap::DrawEllipseLines(RefPtr<Vector2> center,
                              float radius_h,
                              float radius_v,
                              RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(center);
  CHECK_VALUE(color);

  ShapeDrawScope scope(this);
  raylib::DrawEllipseLinesV(center->As(), radius_h, radius_v,
                            PremultiplyColor(color));
}

void Bitmap::DrawRing(RefPtr<Vector2> center,
                      float inner_radius,
                      float outer_radius,
                      float start_angle,
                      float end_angle,
                      int segments,
                      RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(center);
  CHECK_VALUE(color);

  ShapeDrawScope scope(this);
  raylib::DrawRing(center->As(), inner_radius, outer_radius, start_angle,
                   end_angle, segments, PremultiplyColor(color));
}

void Bitmap::DrawRingLines(RefPtr<Vector2> center,
                           float inner_radius,
                           float outer_radius,
                           float start_angle,
                           float end_angle,
                           int segments,
                           RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(center);
  CHECK_VALUE(color);

  ShapeDrawScope scope(this);
  raylib::DrawRingLines(center->As(), inner_radius, outer_radius, start_angle,
                        end_angle, segments, PremultiplyColor(color));
}

// ---------------------------------------------------------------------------
// Splines drawing functions (raylib passthrough)
// ---------------------------------------------------------------------------

void Bitmap::DrawSplineLinear(std::vector<RefPtr<Vector2>> points,
                              float thick,
                              RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(color);

  auto rpoints = ToRaylibPoints(points);
  ShapeDrawScope scope(this);
  raylib::DrawSplineLinear(rpoints.data(), (int)rpoints.size(), thick,
                           PremultiplyColor(color));
}

void Bitmap::DrawSplineBasis(std::vector<RefPtr<Vector2>> points,
                             float thick,
                             RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(color);

  auto rpoints = ToRaylibPoints(points);
  ShapeDrawScope scope(this);
  raylib::DrawSplineBasis(rpoints.data(), (int)rpoints.size(), thick,
                          PremultiplyColor(color));
}

void Bitmap::DrawSplineCatmullRom(std::vector<RefPtr<Vector2>> points,
                                  float thick,
                                  RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(color);

  auto rpoints = ToRaylibPoints(points);
  ShapeDrawScope scope(this);
  raylib::DrawSplineCatmullRom(rpoints.data(), (int)rpoints.size(), thick,
                               PremultiplyColor(color));
}

void Bitmap::DrawSplineBezierQuadratic(std::vector<RefPtr<Vector2>> points,
                                       float thick,
                                       RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(color);

  auto rpoints = ToRaylibPoints(points);
  ShapeDrawScope scope(this);
  raylib::DrawSplineBezierQuadratic(rpoints.data(), (int)rpoints.size(), thick,
                                    PremultiplyColor(color));
}

void Bitmap::DrawSplineBezierCubic(std::vector<RefPtr<Vector2>> points,
                                   float thick,
                                   RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(color);

  auto rpoints = ToRaylibPoints(points);
  ShapeDrawScope scope(this);
  raylib::DrawSplineBezierCubic(rpoints.data(), (int)rpoints.size(), thick,
                                PremultiplyColor(color));
}

void Bitmap::DrawSplineSegmentLinear(RefPtr<Vector2> p1,
                                     RefPtr<Vector2> p2,
                                     float thick,
                                     RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(p1);
  CHECK_VALUE(p2);
  CHECK_VALUE(color);

  ShapeDrawScope scope(this);
  raylib::DrawSplineSegmentLinear(p1->As(), p2->As(), thick,
                                  PremultiplyColor(color));
}

void Bitmap::DrawSplineSegmentBasis(RefPtr<Vector2> p1,
                                    RefPtr<Vector2> p2,
                                    RefPtr<Vector2> p3,
                                    RefPtr<Vector2> p4,
                                    float thick,
                                    RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(p1);
  CHECK_VALUE(p2);
  CHECK_VALUE(p3);
  CHECK_VALUE(p4);
  CHECK_VALUE(color);

  ShapeDrawScope scope(this);
  raylib::DrawSplineSegmentBasis(p1->As(), p2->As(), p3->As(), p4->As(), thick,
                                 PremultiplyColor(color));
}

void Bitmap::DrawSplineSegmentCatmullRom(RefPtr<Vector2> p1,
                                         RefPtr<Vector2> p2,
                                         RefPtr<Vector2> p3,
                                         RefPtr<Vector2> p4,
                                         float thick,
                                         RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(p1);
  CHECK_VALUE(p2);
  CHECK_VALUE(p3);
  CHECK_VALUE(p4);
  CHECK_VALUE(color);

  ShapeDrawScope scope(this);
  raylib::DrawSplineSegmentCatmullRom(p1->As(), p2->As(), p3->As(), p4->As(),
                                      thick, PremultiplyColor(color));
}

void Bitmap::DrawSplineSegmentBezierQuadratic(RefPtr<Vector2> p1,
                                              RefPtr<Vector2> c2,
                                              RefPtr<Vector2> p3,
                                              float thick,
                                              RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(p1);
  CHECK_VALUE(c2);
  CHECK_VALUE(p3);
  CHECK_VALUE(color);

  ShapeDrawScope scope(this);
  raylib::DrawSplineSegmentBezierQuadratic(p1->As(), c2->As(), p3->As(), thick,
                                           PremultiplyColor(color));
}

void Bitmap::DrawSplineSegmentBezierCubic(RefPtr<Vector2> p1,
                                          RefPtr<Vector2> c2,
                                          RefPtr<Vector2> c3,
                                          RefPtr<Vector2> p4,
                                          float thick,
                                          RefPtr<Color> color) {
  Dispoable::Guard();
  CHECK_VALUE(p1);
  CHECK_VALUE(c2);
  CHECK_VALUE(c3);
  CHECK_VALUE(p4);
  CHECK_VALUE(color);

  ShapeDrawScope scope(this);
  raylib::DrawSplineSegmentBezierCubic(p1->As(), c2->As(), c3->As(), p4->As(),
                                       thick, PremultiplyColor(color));
}

ATTR_DEF(RefPtr<Font>, Font, Bitmap) {
  if (value.has_value()) {
    CHECK_VALUE(*value);

    font_ = MakeRefCounted<Font>(*value);
    return std::nullopt;
  } else {
    return font_;
  }
}

ATTR_DEF(RefPtr<Bitmap>, ShapeBitmap, Bitmap) {
  if (value.has_value()) {
    CHECK_VALUE((shape_bitmap_.get() != this));

    shape_bitmap_ = *value;
    return std::nullopt;
  } else {
    return shape_bitmap_;
  }
}

void Bitmap::DisposeObject() {
  raylib::UnloadRenderTexture(texture_);

  texture_ = {};
}

}  // namespace lime
