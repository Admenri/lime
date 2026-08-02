#pragma once

#include "mruby_utils.h"

namespace binding {

MRB_DATATYPE_DECLARE(Window);

void InitWindowBinding(mrb_state* mrb);

}  // namespace binding
