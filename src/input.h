#pragma once

#include "common.h"

namespace rgssx {

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

  // sym -> keycode
  using KeySym = std::pair<std::string, int>;
  void SetKeyBinding(std::vector<KeySym> bindings) { bindings_ = bindings_; }

  void Update();
  bool Pressed(std::string sym);
  bool Triggered(std::string sym);
  bool Repeated(std::string sym);
  int Dir4();
  int Dir8();

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

}  // namespace rgssx