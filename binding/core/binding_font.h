#pragma once

#include "mruby_utils.h"

namespace binding {

MRB_DATATYPE_DECLARE(Font);

void InitFontBinding(mrb_state* mrb);

}  // namespace binding
