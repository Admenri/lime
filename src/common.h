#pragma once

#include <exception>
#include <format>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace raylib {
#include "raylib.h"
#include "rlgl.h"

// HOOK raylib y flipping otrho
inline void BeginTextureRender(RenderTexture2D target) {
  raylib::BeginTextureMode(target);

  // invert vertical raylib internal projection
  raylib::rlMatrixMode(RL_PROJECTION);
  raylib::rlLoadIdentity();
  raylib::rlOrtho(0, target.texture.width, 0, target.texture.height, 0.0, 1.0);
}
#define BeginTextureMode BeginTextureRender

inline void EndBlendModePremultiply() {
  rlSetBlendMode(RL_BLEND_ALPHA_PREMULTIPLY);
}
#define EndBlendMode EndBlendModePremultiply

inline Color MakeAlphaColor(uint8_t alpha) {
  return Color{alpha, alpha, alpha, alpha};
}

}  // namespace raylib

#define ATTR(ty, name) \
  std::optional<ty> Attr_##name(std::optional<ty> value = std::nullopt)
#define ATTR_DEF(ty, name, klass) \
  std::optional<ty> klass::Attr_##name(std::optional<ty> value)

namespace rgssx {

template <typename Ty>
class Singleton {
 public:
  static Ty* Instance() { return instance_.get(); }
  static void Instance(Ty* instance) { instance_.reset(instance); }

 private:
  inline static std::unique_ptr<Ty> instance_;
};

class Exception : public std::exception {
 public:
  template <typename... Args>
  explicit Exception(std::string_view format, Args&&... args) {
    message_ = std::vformat(format,
                            std::make_format_args(std::forward<Args>(args)...));
  }

  // std::exception::what
  const char* what() const override { return message_.c_str(); }

 private:
  std::string message_;
};

class Dispoable {
 public:
  bool IsDisposed() { return disposed_; }
  void Dispose() {
    if (!disposed_) {
      DisposeObject();
      disposed_ = true;
    }
  }

 protected:
  void Guard() {
    if (!disposed_)
      return;

    // disposed error
    throw Exception("disposed object");
  }

  virtual void DisposeObject() = 0;

 private:
  bool disposed_ = false;
};

}  // namespace rgssx
