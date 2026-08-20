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

#pragma once

#include <format>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "src/define.h"

namespace lime {

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

}  // namespace lime
