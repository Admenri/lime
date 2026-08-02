#pragma once

#include "mruby_utils.h"

namespace binding {

// Graphics is bound as a Ruby module (module functions access the singleton
// through Graphics::Instance()).
void InitGraphicsBinding(mrb_state* mrb);

}  // namespace binding
