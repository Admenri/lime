#pragma once

#include <format>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "src/define.h"

namespace rgssx {

template <typename Ty>
class Singleton {
 public:
  static Ty* Instance() { return instance_.get(); }
  static void Instance(Ty* instance) { instance_.reset(instance); }

 private:
  inline static std::unique_ptr<Ty> instance_;
};

class Exception final {
 public:
  enum Type {
    ExitError = 0,
    ResetError,
    RGSSError,
    IOError,
  };

  template <typename... Args>
  explicit Exception(Type type, std::string_view format, Args&&... args) {
    message_ = std::vformat(format,
                            std::make_format_args(std::forward<Args>(args)...));
  }

  Type type() const noexcept { return type_; }
  std::string message() const noexcept { return message_; }

 private:
  Type type_ = {};
  std::string message_;
};

class Dispoable {
 public:
  /*-export.begin-*/
  bool IsDisposed() { return disposed_; }
  void Dispose() {
    if (!disposed_) {
      DisposeObject();
      disposed_ = true;
    }
  }
  /*-export.end-*/

 protected:
  void Guard() {
    if (!disposed_)
      return;

    // disposed error
    throw Exception(Exception::RGSSError, "disposed object");
  }

  virtual void DisposeObject() = 0;

 private:
  bool disposed_ = false;
};

}  // namespace rgssx
