#pragma once

#include "mruby_utils.h"

namespace binding {

MRB_DATATYPE_DECLARE(Plane);

void InitPlaneBinding(mrb_state* mrb);

}  // namespace binding
