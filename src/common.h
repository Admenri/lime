#pragma once

#include <exception>
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
    throw Exception("disposed object");
  }

  virtual void DisposeObject() = 0;

 private:
  bool disposed_ = false;
};

}  // namespace rgssx
