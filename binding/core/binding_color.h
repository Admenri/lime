#pragma once

#include "mruby_utils.h"

namespace binding {

MRB_DATATYPE_DECLARE(Color);

void InitColorBinding(mrb_state* mrb);

}  // namespace binding
