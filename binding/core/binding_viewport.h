#pragma once

#include "mruby_utils.h"

namespace binding {

MRB_DATATYPE_DECLARE(Viewport);

void InitViewportBinding(mrb_state* mrb);

}  // namespace binding
