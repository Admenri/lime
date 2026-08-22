#pragma once

#include "mruby_utils.h"

namespace binding {

MRB_DATATYPE_DECLARE(WindowXP);

void InitWindowXPBinding(mrb_state* mrb);

}  // namespace binding