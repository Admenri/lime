#include "src/input.h"

#include "src/raywarp.h"

namespace lime {

const Input::KeySym kDefaultKeyboardBindings[] = {
    {"DOWN", raylib::KEY_DOWN},
    {"LEFT", raylib::KEY_LEFT},
    {"RIGHT", raylib::KEY_RIGHT},
    {"UP", raylib::KEY_UP},

    {"F5", raylib::KEY_F5},
    {"F6", raylib::KEY_F6},
    {"F7", raylib::KEY_F7},
    {"F8", raylib::KEY_F8},
    {"F9", raylib::KEY_F9},

    {"SHIFT", raylib::KEY_LEFT_SHIFT},
    {"SHIFT", raylib::KEY_RIGHT_SHIFT},
    {"CTRL", raylib::KEY_LEFT_CONTROL},
    {"CTRL", raylib::KEY_RIGHT_CONTROL},
    {"ALT", raylib::KEY_LEFT_ALT},
    {"ALT", raylib::KEY_RIGHT_ALT},

    {"A", raylib::KEY_LEFT_SHIFT},
    {"B", raylib::KEY_ESCAPE},
    {"B", raylib::KEY_KP_0},
    {"B", raylib::KEY_X},
    {"C", raylib::KEY_SPACE},
    {"C", raylib::KEY_ENTER},
    {"X", raylib::KEY_A},
    {"Y", raylib::KEY_S},
    {"Z", raylib::KEY_D},
    {"L", raylib::KEY_Q},
    {"R", raylib::KEY_W},
};

const Input::KeySym kKeyboardBindings1[] = {
    {"A", raylib::KEY_Z},
    {"C", raylib::KEY_C},
};

const Input::KeySym kKeyboardBindings2[] = {
    {"C", raylib::KEY_Z},
};

const std::string kArrowDirsSymbol[] = {
    "DOWN",
    "LEFT",
    "RIGHT",
    "UP",
};

const std::string kButtonItems[] = {
    "A", "B", "C", "X", "Y", "Z", "L", "R", "DOWN", "LEFT", "RIGHT", "UP",
};

Input::Input(int version) {
  for (size_t i = 0; i < std::size(kDefaultKeyboardBindings); ++i)
    bindings_.push_back(kDefaultKeyboardBindings[i]);

  // == XP
  if (version == 1)
    for (size_t i = 0; i < std::size(kKeyboardBindings1); ++i)
      bindings_.push_back(kKeyboardBindings1[i]);

  // >= VX
  if (version >= 2)
    for (size_t i = 0; i < std::size(kKeyboardBindings2); ++i)
      bindings_.push_back(kKeyboardBindings2[i]);
}

Input::~Input() = default;

void Input::Update() {
  for (int i = 0; i < std::size(states_); ++i) {
    bool key_pressed = raylib::IsKeyDown(i);

    // Update key state with elder state
    states_[i].trigger = !states_[i].pressed && key_pressed;

    // After trigger set, set press state
    states_[i].pressed = key_pressed;

    // Based on press state update the repeat state
    states_[i].repeat = false;
    if (states_[i].pressed) {
      ++states_[i].repeat_count;

      bool repeated = false;
      // TODO: RGSS 1/2/3 specific process
      repeated = states_[i].repeat_count == 1 ||
                 (states_[i].repeat_count >= 23 &&
                  (states_[i].repeat_count + 1) % 6 == 0);

      states_[i].repeat = repeated;
    } else {
      states_[i].repeat_count = 0;
    }
  }

  UpdateDir4();
  UpdateDir8();
}

bool Input::Pressed(std::string sym) {
  if (sym.empty())
    return false;

  for (auto& it : bindings_) {
    if (it.first == sym)
      if (states_[it.second].pressed)
        return true;
  }

  return false;
}

bool Input::Triggered(std::string sym) {
  if (sym.empty())
    return false;

  for (auto& it : bindings_) {
    if (it.first == sym)
      if (states_[it.second].trigger)
        return true;
  }

  return false;
}

bool Input::Repeated(std::string sym) {
  if (sym.empty())
    return false;

  for (auto& it : bindings_) {
    if (it.first == sym)
      if (states_[it.second].repeat)
        return true;
  }

  return false;
}

int Input::Dir4() {
  return dir4_state_.active;
}

int Input::Dir8() {
  return dir8_state_.active;
}

void Input::UpdateDir4() {
  bool key_states[std::size(kArrowDirsSymbol)] = {0};
  for (auto& it : bindings_)
    for (size_t i = 0; i < std::size(kArrowDirsSymbol); ++i)
      if (it.first == kArrowDirsSymbol[i])
        key_states[i] |= states_[it.second].pressed;

  int dir_flag = 0;
  const int dir_flags_fix[] = {
      1 << 1,
      1 << 2,
      1 << 3,
      1 << 4,
  };

  const int block_dir_flags[] = {dir_flags_fix[0] | dir_flags_fix[3],
                                 dir_flags_fix[1] | dir_flags_fix[2]};

  const int other_dirs[][3] = {
      {1, 2, 3},
      {0, 3, 2},
      {0, 3, 1},
      {1, 2, 0},
  };

  for (size_t i = 0; i < 4; ++i)
    dir_flag |= (key_states[i] ? dir_flags_fix[i] : 0);

  if (dir_flag == block_dir_flags[0] || dir_flag == block_dir_flags[1]) {
    dir4_state_.active = 0;
    return;
  }

  if (dir4_state_.previous) {
    if (key_states[dir4_state_.previous / 2 - 1]) {
      for (size_t i = 0; i < 3; ++i) {
        int other_key = other_dirs[dir4_state_.previous / 2 - 1][i];
        if (!key_states[other_key])
          continue;

        dir4_state_.active = (other_key + 1) * 2;
        return;
      }
    }
  }

  for (int i = 0; i < 4; ++i) {
    if (!key_states[i])
      continue;

    dir4_state_.active = (i + 1) * 2;
    dir4_state_.previous = (i + 1) * 2;
    return;
  }

  dir4_state_.active = 0;
  dir4_state_.previous = 0;
}

void Input::UpdateDir8() {
  bool key_states[std::size(kArrowDirsSymbol)] = {0};
  for (auto& it : bindings_)
    for (size_t i = 0; i < std::size(kArrowDirsSymbol); ++i)
      if (it.first == kArrowDirsSymbol[i])
        key_states[i] |= states_[it.second].pressed;

  static const int combos[4][4] = {
      {2, 1, 3, 0}, {1, 4, 0, 7}, {3, 0, 6, 9}, {0, 7, 9, 8}};

  const int other_dirs[][3] = {
      {1, 2, 3},
      {0, 3, 2},
      {0, 3, 1},
      {1, 2, 0},
  };

  dir8_state_.active = 0;

  for (int i = 0; i < 4; ++i) {
    if (!key_states[i])
      continue;

    for (int j = 0; j < 3; ++j) {
      int other_key = other_dirs[i][j];
      if (!key_states[other_key])
        continue;

      dir8_state_.active = combos[i][other_key];
      return;
    }

    dir8_state_.active = (i + 1) * 2;
    return;
  }
}

}  // namespace lime