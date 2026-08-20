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

#include "src/common.h"

namespace lime {

inline const struct {
  std::string name;
  int key_id;
} kKeyboardBindings[] = {
    {"DOWN", 2},   {"LEFT", 4},  {"RIGHT", 6}, {"UP", 8},

    {"A", 11},     {"B", 12},    {"C", 13},    {"X", 14},  {"Y", 15},
    {"Z", 16},     {"L", 17},    {"R", 18},

    {"SHIFT", 21}, {"CTRL", 22}, {"ALT", 23},

    {"F5", 25},    {"F6", 26},   {"F7", 27},   {"F8", 28}, {"F9", 29},
};

class Input : public Singleton<Input> {
 public:
  Input(int version);
  ~Input();

  /*-export.begin-*/
  void Update();
  bool Pressed(std::string sym);
  bool Triggered(std::string sym);
  bool Repeated(std::string sym);
  int Dir4();
  int Dir8();

  bool KeyPressed(int keycode);
  bool KeyTriggered(int keycode);
  bool KeyRepeated(int keycode);
  /*-export.end-*/

 public:
  // sym -> keycode
  using KeySym = std::pair<std::string, int>;
  void SetKeyBinding(std::vector<KeySym> bindings) { bindings_ = bindings_; }

 private:
  void UpdateDir4();
  void UpdateDir8();

  struct {
    bool pressed = false;
    bool trigger = false;
    bool repeat = false;
    int repeat_count = 0;
  } states_[512];

  struct {
    int active = 0;
    int previous = 0;
  } dir4_state_;

  struct {
    int active = 0;
  } dir8_state_;

  std::vector<KeySym> bindings_;
};

}  // namespace lime