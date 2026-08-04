#include "src/bitmap.h"

#include "src/filesystem.h"

namespace rgssx {

Bitmap::Bitmap(std::string filename) : font_(MakeRefCounted<Font>()) {
  raylib::Image image = {};

  // RGSS style loading: the extension may be omitted and the virtual file
  // system will resolve the actual file (e.g. "Iconset" -> "Iconset.png").
  IOService::Instance()->OpenRead(
      filename,
      [&](std::unique_ptr<std::istream> stream, const std::string& ext) {
        std::string data = ReadStream(*stream);

        // raylib needs a file type hint (e.g. ".png")
        std::string file_type = "." + ext;
        image = raylib::LoadImageFromMemory(file_type.c_str(),
                                            (const unsigned char*)data.data(),
                                            (int)data.size());
        return true;  // matched, stop enumeration
      });

  if (!image.data)
    throw Exception("failed to load image: {}", filename);

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
  // TODO
}

void Bitmap::DrawText(RefPtr<Rect> rect, std::string str, int align) {
  Dispoable::Guard();
  DrawText(rect->x, rect->y, rect->width, rect->height, str, align);
}

RefPtr<Rect> Bitmap::TextSize(std::string str) {
  Dispoable::Guard();
  // TODO
  return MakeRefCounted<Rect>();
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

}  // namespace rgssx
