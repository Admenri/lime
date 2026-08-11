#pragma once

#include "mruby_utils.h"

namespace binding {

MRB_DATATYPE_DECLARE(Vector2);
MRB_DATATYPE_DECLARE(Vector3);
MRB_DATATYPE_DECLARE(Vector4);

void InitVectorBinding(mrb_state* mrb);

}  // namespace binding
