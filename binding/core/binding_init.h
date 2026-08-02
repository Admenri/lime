#pragma once

#include "mruby.h"

namespace binding {

// Initializes all generated class bindings. Call once after mrb_open().
void InitBindings(mrb_state* mrb);

}  // namespace binding
