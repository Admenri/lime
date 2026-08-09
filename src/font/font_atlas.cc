// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/font/font_atlas.h"

#include <algorithm>
#include <cstring>
#include <unordered_map>

namespace lime::font {
namespace {

// Pack (width, height) into a 64-bit key for the exact-size free list.
inline uint64_t PackSize(int w, int h) {
  return (static_cast<uint64_t>(static_cast<uint32_t>(w)) << 32) |
         static_cast<uint32_t>(h);
}

// ---------------------------------------------------------------------------
// Segment-based skyline packer (bottom-left fit, Jylänki-style).
//
// The skyline is a piecewise-constant function over x, stored as horizontal
// spans (x, y, width) sorted by x with no overlap and no adjacent spans of
// equal height. Allocate() scans for the lowest baseline that fits a w-wide
// rectangle, then replaces the covered skyline region with a constant span.
// ---------------------------------------------------------------------------
class Skyline {
 public:
  Skyline() = default;
  Skyline(int width, int height) { Reset(width, height); }

  void Reset(int width, int height) {
    width_ = width;
    height_ = height;
    spans_.clear();
    spans_.push_back({0, 0, width});
  }

  bool Allocate(int w, int h, int& out_x, int& out_y) {
    if (w <= 0 || h <= 0 || w > width_ || h > height_) return false;

    int best_index = -1;
    int best_x = 0;
    int best_y = height_ + 1;
    int best_waste = 0;

    for (int i = 0; i < static_cast<int>(spans_.size()); ++i) {
      const int x = spans_[i].x;
      if (x + w > width_) continue;

      // Lowest baseline in the window [x, x + w).
      int y = spans_[i].y;
      int j = i;
      while (j < static_cast<int>(spans_.size()) && spans_[j].x < x + w) {
        y = std::max(y, spans_[j].y);
        ++j;
      }
      if (y + h > height_) continue;

      // Waste = skyline area above the starting baseline inside the window.
      int waste = 0;
      for (int k = i; k < j; ++k) {
        const int overlap =
            std::min(spans_[k].x + spans_[k].width, x + w) -
            std::max(spans_[k].x, x);
        waste += overlap * std::max(0, spans_[k].y - spans_[i].y);
      }
      if (y < best_y || (y == best_y && waste < best_waste)) {
        best_index = i;
        best_x = x;
        best_y = y;
        best_waste = waste;
      }
    }

    if (best_index < 0) return false;

    Insert(best_x, best_y + h, w);
    out_x = best_x;
    out_y = best_y;
    return true;
  }

 private:
  struct Span {
    int x, y, width;
  };

  int NodeIndexAt(int x) const {
    for (int i = 0; i < static_cast<int>(spans_.size()); ++i)
      if (spans_[i].x >= x) return i;
    return static_cast<int>(spans_.size());
  }

  // Ensure a span boundary exists at x.
  void SplitAt(int x) {
    if (x <= 0 || x >= width_) return;
    for (int i = 0; i < static_cast<int>(spans_.size()); ++i) {
      const int nx = spans_[i].x;
      const int nw = spans_[i].width;
      if (nx < x && nx + nw > x) {
        spans_[i].width = x - nx;
        spans_.insert(spans_.begin() + i + 1, {x, spans_[i].y, nw - (x - nx)});
        return;
      }
      if (nx >= x) return;
    }
  }

  void MergeAdjacent() {
    for (int i = 0; i + 1 < static_cast<int>(spans_.size());) {
      if (spans_[i].y == spans_[i + 1].y) {
        spans_[i].width += spans_[i + 1].width;
        spans_.erase(spans_.begin() + i + 1);
      } else {
        ++i;
      }
    }
  }

  // Replace the skyline over [x, x + w) with the constant value `top`.
  void Insert(int x, int top, int w) {
    SplitAt(x);
    const int idx = NodeIndexAt(x);
    SplitAt(x + w);
    const int end_idx = NodeIndexAt(x + w);
    spans_.erase(spans_.begin() + idx, spans_.begin() + end_idx);
    spans_.insert(spans_.begin() + idx, {x, top, w});
    MergeAdjacent();
  }

  int width_ = 0;
  int height_ = 0;
  std::vector<Span> spans_;
};

}  // namespace

struct FontAtlas::Page {
  int width = 0;
  int height = 0;
  raylib::Texture2D texture = {};
  std::vector<uint8_t> pixels;  // RGBA8 CPU mirror of the GPU texture
  Skyline skyline;
  size_t used = 0;  // pixels currently owned by live glyphs
  // Exact-size holes left by evicted glyphs (reused by Allocate()).
  std::unordered_map<uint64_t, std::vector<Allocation>> free_list;
};

FontAtlas::FontAtlas(int page_width, int page_height, int max_pages,
                     bool premultiplied_alpha)
    : page_width_(page_width),
      page_height_(page_height),
      max_pages_(max_pages),
      premultiplied_alpha_(premultiplied_alpha) {}

FontAtlas::~FontAtlas() {
  for (auto& page : pages_)
    if (raylib::IsTextureValid(page->texture))
      raylib::UnloadTexture(page->texture);
}

FontAtlas::Page* FontAtlas::CreatePage() {
  auto page = std::make_unique<Page>();
  page->width = page_width_;
  page->height = page_height_;
  page->pixels.assign(static_cast<size_t>(page_width_) * page_height_ * 4, 0);
  page->skyline.Reset(page_width_, page_height_);

  // Build a texture from the zeroed CPU buffer.
  raylib::Image image = {};
  image.data = page->pixels.data();
  image.width = page_width_;
  image.height = page_height_;
  image.mipmaps = 1;
  image.format = raylib::PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
  page->texture = raylib::LoadTextureFromImage(image);
  raylib::SetTextureFilter(page->texture, raylib::TEXTURE_FILTER_BILINEAR);
  raylib::SetTextureWrap(page->texture, raylib::TEXTURE_WRAP_CLAMP);

  pages_.push_back(std::move(page));
  return pages_.back().get();
}

bool FontAtlas::Allocate(int width, int height, Allocation& out) {
  if (width <= 0 || height <= 0) return false;
  const uint64_t key = PackSize(width, height);

  // 1) Reuse an exact-size hole left by eviction.
  for (size_t i = 0; i < pages_.size(); ++i) {
    Page& page = *pages_[i];
    auto it = page.free_list.find(key);
    if (it != page.free_list.end() && !it->second.empty()) {
      out = it->second.back();
      it->second.pop_back();
      if (it->second.empty()) page.free_list.erase(it);
      page.used += static_cast<size_t>(width) * height;
      return true;
    }
  }

  // 2) Skyline pack into an existing page (first page that fits).
  for (size_t i = 0; i < pages_.size(); ++i) {
    Page& page = *pages_[i];
    int x = 0, y = 0;
    if (page.skyline.Allocate(width, height, x, y)) {
      out = {static_cast<int>(i), x, y, width, height};
      page.used += static_cast<size_t>(width) * height;
      return true;
    }
  }

  // 3) Grow a new page.
  if (static_cast<int>(pages_.size()) < max_pages_) {
    Page* page = CreatePage();
    int x = 0, y = 0;
    if (page->skyline.Allocate(width, height, x, y)) {
      out = {static_cast<int>(pages_.size() - 1), x, y, width, height};
      page->used += static_cast<size_t>(width) * height;
      return true;
    }
  }
  return false;
}

void FontAtlas::Free(const Allocation& alloc) {
  if (alloc.page < 0 || alloc.page >= static_cast<int>(pages_.size())) return;
  if (alloc.width <= 0 || alloc.height <= 0) return;

  Page& page = *pages_[alloc.page];
  // Zero the whole region (border included) on CPU + GPU.
  std::vector<uint8_t> zeros(static_cast<size_t>(alloc.width) * alloc.height * 4,
                             0);
  UploadRegion(page, alloc.x, alloc.y, alloc.width, alloc.height, zeros.data());
  // Make the hole reusable for the same size.
  page.free_list[PackSize(alloc.width, alloc.height)].push_back(alloc);
  page.used -= std::min(page.used, static_cast<size_t>(alloc.width) * alloc.height);
}

void FontAtlas::UploadRegion(Page& page, int x, int y, int w, int h,
                             const unsigned char* rgba) {
  if (x < 0 || y < 0 || x + w > page.width || y + h > page.height || !rgba)
    return;
  const size_t row_bytes = static_cast<size_t>(w) * 4;
  for (int row = 0; row < h; ++row) {
    std::memcpy(&page.pixels[(static_cast<size_t>(y + row) * page.width + x) * 4],
                rgba + static_cast<size_t>(row) * row_bytes, row_bytes);
  }
  // Only the newly written region reaches the GPU (requirement: no full
  // texture re-upload).
  raylib::UpdateTextureRec(
      page.texture,
      {static_cast<float>(x), static_cast<float>(y), static_cast<float>(w),
       static_cast<float>(h)},
      rgba);
}

void FontAtlas::UploadRGBA(const Allocation& alloc, const unsigned char* rgba,
                           int src_width, int src_height, int src_pitch) {
  if (alloc.page < 0 || alloc.page >= static_cast<int>(pages_.size())) return;
  Page& page = *pages_[alloc.page];

  std::vector<uint8_t> packed(static_cast<size_t>(src_width) * src_height * 4);
  for (int row = 0; row < src_height; ++row)
    std::memcpy(&packed[static_cast<size_t>(row) * src_width * 4],
                rgba + static_cast<size_t>(row) * src_pitch,
                static_cast<size_t>(src_width) * 4);
  UploadRegion(page, alloc.x, alloc.y, src_width, src_height, packed.data());
}

void FontAtlas::UploadGray(const Allocation& alloc, const unsigned char* gray,
                           int src_width, int src_height, int src_pitch) {
  if (alloc.page < 0 || alloc.page >= static_cast<int>(pages_.size())) return;
  Page& page = *pages_[alloc.page];

  // Grayscale -> white glyph. Premultiplied stores (a, a, a, a); straight
  // alpha stores (255, 255, 255, a).
  std::vector<uint8_t> rgba(static_cast<size_t>(src_width) * src_height * 4);
  for (int row = 0; row < src_height; ++row) {
    const unsigned char* src = gray + static_cast<size_t>(row) * src_pitch;
    uint8_t* dst = &rgba[static_cast<size_t>(row) * src_width * 4];
    for (int col = 0; col < src_width; ++col) {
      const uint8_t a = src[col];
      if (premultiplied_alpha_) {
        dst[col * 4 + 0] = a;
        dst[col * 4 + 1] = a;
        dst[col * 4 + 2] = a;
      } else {
        dst[col * 4 + 0] = 255;
        dst[col * 4 + 1] = 255;
        dst[col * 4 + 2] = 255;
      }
      dst[col * 4 + 3] = a;
    }
  }
  // Place inside the allocation, leaving a 1px transparent border.
  UploadRegion(page, alloc.x + 1, alloc.y + 1, src_width, src_height,
               rgba.data());
}

void FontAtlas::UploadColor(const Allocation& alloc, const unsigned char* bgra,
                            int src_width, int src_height, int src_pitch) {
  if (alloc.page < 0 || alloc.page >= static_cast<int>(pages_.size())) return;
  Page& page = *pages_[alloc.page];

  // FreeType FT_PIXEL_MODE_BGRA (straight alpha) -> RGBA. Premultiplied
  // multiplies the color channels by alpha; straight alpha keeps them as-is.
  std::vector<uint8_t> rgba(static_cast<size_t>(src_width) * src_height * 4);
  for (int row = 0; row < src_height; ++row) {
    const unsigned char* src = bgra + static_cast<size_t>(row) * src_pitch;
    uint8_t* dst = &rgba[static_cast<size_t>(row) * src_width * 4];
    for (int col = 0; col < src_width; ++col) {
      const int s = col * 4;
      if (premultiplied_alpha_) {
        const float a = src[s + 3] / 255.0f;
        dst[s + 0] = static_cast<uint8_t>(src[s + 2] * a);  // R
        dst[s + 1] = static_cast<uint8_t>(src[s + 1] * a);  // G
        dst[s + 2] = static_cast<uint8_t>(src[s + 0] * a);  // B
      } else {
        dst[s + 0] = src[s + 2];  // R
        dst[s + 1] = src[s + 1];  // G
        dst[s + 2] = src[s + 0];  // B
      }
      dst[s + 3] = src[s + 3];    // A
    }
  }
  UploadRegion(page, alloc.x + 1, alloc.y + 1, src_width, src_height,
               rgba.data());
}

raylib::Texture2D FontAtlas::GetTexture(int page) const {
  if (page < 0 || page >= static_cast<int>(pages_.size())) return {};
  return pages_[static_cast<size_t>(page)]->texture;
}

float FontAtlas::Usage(int page) const {
  if (page < 0 || page >= static_cast<int>(pages_.size())) return 0.f;
  const size_t capacity =
      static_cast<size_t>(page_width_) * static_cast<size_t>(page_height_);
  return capacity ? static_cast<float>(pages_[static_cast<size_t>(page)]->used) /
                        static_cast<float>(capacity)
                  : 0.f;
}

void FontAtlas::Clear() {
  for (auto& page : pages_) {
    std::fill(page->pixels.begin(), page->pixels.end(), 0);
    if (raylib::IsTextureValid(page->texture))
      raylib::UpdateTexture(page->texture, page->pixels.data());
    page->skyline.Reset(page_width_, page_height_);
    page->free_list.clear();
    page->used = 0;
  }
}

}  // namespace lime::font
