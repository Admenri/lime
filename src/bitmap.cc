#include "src/bitmap.h"

#include "src/filesystem.h"
#include "src/font/font_system.h"

namespace lime {

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
  texture_ = raylib::LoadRenderTexture(other->Width(), other->Height());

  raylib::BeginTextureMode(texture_);
  raylib::rlDisableColorBlend();
  raylib::DrawTexture(other->render_texture().texture, 0, 0, raylib::RAYWHITE);
  raylib::EndTextureMode();
}

Bitmap::~Bitmap() {
  Dispose();
}

int Bitmap::Width() {
  return texture_.texture.width;
}

int Bitmap::Height() {
  return texture_.texture.height;
}

RefPtr<Rect> Bitmap::GetRect() {
  return MakeRefCounted<Rect>(0, 0, Width(), Height());
}

void Bitmap::Blt(int x,
                 int y,
                 RefPtr<Bitmap> src_bitmap,
                 RefPtr<Rect> src_rect,
                 int opacity) {
  Dispoable::Guard();
  StretchBlt(MakeRefCounted<Rect>(x, y, src_rect->width, src_rect->height),
             src_bitmap, src_rect, opacity);
}

void Bitmap::StretchBlt(RefPtr<Rect> dst_rect,
                        RefPtr<Bitmap> src_bitmap,
                        RefPtr<Rect> src_rect,
                        int opacity) {
  Dispoable::Guard();
  raylib::BeginTextureMode(texture_);
  raylib::rlEnableColorBlend();
  raylib::rlSetBlendMode(raylib::BLEND_ALPHA_PREMULTIPLY);
  raylib::DrawTexturePro(src_bitmap->render_texture().texture, src_rect->As(),
                         dst_rect->As(), {}, 0,
                         raylib::MakeColor(std::clamp<int>(opacity, 0, 255)));
  raylib::EndTextureMode();
}

void Bitmap::FillRect(int x,
                      int y,
                      int width,
                      int height,
                      RefPtr<Color> color) {
  Dispoable::Guard();
  raylib::BeginTextureMode(texture_);
  raylib::rlDisableColorBlend();
  raylib::DrawRectangle(x, y, width, height, color->As());
  raylib::EndTextureMode();
}

void Bitmap::FillRect(RefPtr<Rect> rect, RefPtr<Color> color) {
  Dispoable::Guard();
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
  raylib::BeginTextureMode(texture_);
  raylib::rlDisableColorBlend();
  if (vertical) {
    raylib::DrawRectangleGradientV(x, y, width, height, color1->As(),
                                   color2->As());
  } else {
    raylib::DrawRectangleGradientH(x, y, width, height, color1->As(),
                                   color2->As());
  }
  raylib::EndTextureMode();
}

void Bitmap::GradientFillRect(RefPtr<Rect> rect,
                              RefPtr<Color> color1,
                              RefPtr<Color> color2,
                              bool vertical) {
  Dispoable::Guard();
  GradientFillRect(rect->x, rect->y, rect->width, rect->height, color1, color2,
                   vertical);
}

void Bitmap::Clear() {
  Dispoable::Guard();
  raylib::BeginTextureMode(texture_);
  raylib::ClearBackground({});
  raylib::EndTextureMode();
}

void Bitmap::ClearRect(int x, int y, int width, int height) {
  Dispoable::Guard();
  raylib::BeginTextureMode(texture_);
  raylib::rlDisableColorBlend();
  raylib::DrawRectangle(x, y, width, height, {});
  raylib::EndTextureMode();
}

void Bitmap::ClearRect(RefPtr<Rect> rect) {
  Dispoable::Guard();
  ClearRect(rect->x, rect->y, rect->width, rect->height);
}

RefPtr<Color> Bitmap::GetPixel(int x, int y) {
  Dispoable::Guard();
  auto image = raylib::LoadImageFromTexture(texture_.texture);
  auto color = raylib::GetImageColor(image, x, y);
  raylib::UnloadImage(image);
  return MakeRefCounted<Color>(color);
}

void Bitmap::SetPixel(int x, int y, RefPtr<Color> color) {
  Dispoable::Guard();
  raylib::BeginTextureMode(texture_);
  raylib::rlDisableColorBlend();
  raylib::DrawPixel(x, y, color->As());
  raylib::EndTextureMode();
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
  DrawText(rect->x, rect->y, rect->width, rect->height, str, align);
}

RefPtr<Rect> Bitmap::TextSize(std::string str) {
  Dispoable::Guard();
  if (!font_)
    return MakeRefCounted<Rect>();

  auto& fs = font_->font_system();
  auto size = fs.MeasureText(str.c_str());
  return MakeRefCounted<Rect>(0, 0, static_cast<int>(size.x),
                              static_cast<int>(size.y));
}

void Bitmap::SaveFile(std::string filename) {
  Dispoable::Guard();
  auto image = raylib::LoadImageFromTexture(texture_.texture);
  raylib::ExportImage(image, filename.c_str());
  raylib::UnloadImage(image);
}

ATTR_DEF(RefPtr<Font>, Font, Bitmap) {
  if (value.has_value()) {
    font_ = *value;
    return std::nullopt;
  } else {
    return font_;
  }
}

void Bitmap::DisposeObject() {
  raylib::UnloadRenderTexture(texture_);

  texture_ = {};
}

}  // namespace lime
