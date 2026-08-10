#pragma once

#include "mruby_utils.h"

namespace binding {

MRB_DATATYPE_DECLARE(Geometry);

void InitGeometryBinding(mrb_state* mrb);

}  // namespace binding
