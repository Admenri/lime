#pragma once

#include "mruby_utils.h"

namespace binding {

MRB_DATATYPE_DECLARE(Table);

void InitTableBinding(mrb_state* mrb);

}  // namespace binding
