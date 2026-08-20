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

#pragma once

#include <string>

#include "src/common.h"
#include "src/inirw.h"

namespace lime {

#define g_config Config::Instance()
class Config : public Singleton<Config> {
 public:
  Config(std::string inifile);

  int rgss_version = 0;

  std::string scripts = "Data/Scripts.rvdata2";
  std::string title = "(*^▽^*)";
  std::string rtp;
  std::string soundfont;

  int width = 544;
  int height = 416;
  bool vsync = false;
  bool fullscreen = false;

 public:
  bool xp() { return rgss_version == 1; }
  bool vx() { return rgss_version == 2; }
  bool vxa() { return rgss_version == 3; }

 private:
  IniFile parser_;
};

}  // namespace lime
