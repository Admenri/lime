#include "src/palette.h"

#include "src/filesystem.h"

namespace lime {

Palette::Palette(raylib::Image data) : image_(data) {}

Palette::Palette(int width, int height) {
  raylib::Image image = {};
  image.width = width;
  image.height = height;
  image.data = raylib::MemAlloc(image.width * image.height * 4);
  image.format = raylib::PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
  image.mipmaps = 1;
  std::memset(image.data, 0, image.width * image.height * 4);

  image_ = image;
}

Palette::Palette(std::string filename) {
  // RGSS style loading: the extension may be omitted and the virtual file
  // system will resolve the actual file (e.g. "Iconset" -> "Iconset.png").
  IOService::Instance()->OpenRead(
      filename, [&](std::unique_ptr<IOStream> stream, const std::string& ext) {
        auto data = stream->ReadAll();

        // raylib needs a file type hint (e.g. ".png")
        std::string file_type = "." + ext;
        image_ = raylib::LoadImageFromMemory(
            file_type.c_str(), (uint8_t*)data.data(), (int)data.size());

        return true;  // matched, stop enumeration
      });

  if (!image_.data)
    throw Exception(Exception::RGSSError, "failed to load image: {}", filename);

  raylib::ImageFormat(&image_, raylib::PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
}

Palette::~Palette() {
  Dispose();
}

RefPtr<Color> Palette::GetPixel(int x, int y) {
  Dispoable::Guard();
  auto color = raylib::GetImageColor(image_, x, y);
  return MakeRefCounted<Color>(color);
}

void Palette::SetPixel(int x, int y, RefPtr<Color> color) {
  Dispoable::Guard();

  if (!color)
    throw Exception(Exception::RGSSError, "invalid color.");
  raylib::ImageDrawPixel(&image_, x, y, color->As());
}

void Palette::SaveFile(std::string filename) {
  Dispoable::Guard();
  raylib::ExportImage(image_, filename.c_str());
}

void Palette::DisposeObject() {
  raylib::UnloadImage(image_);
}

}  // namespace lime
