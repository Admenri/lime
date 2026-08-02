#pragma once

#include "mruby_utils.h"

namespace binding {

MRB_DATATYPE_DECLARE(Rect);

void InitRectBinding(mrb_state* mrb);

}  // namespace binding
