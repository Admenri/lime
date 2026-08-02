#pragma once

#include "mruby_utils.h"

namespace binding {

// Input is bound as a Ruby module (module functions access the singleton
// through Input::Instance()).
void InitInputBinding(mrb_state* mrb);

}  // namespace binding
