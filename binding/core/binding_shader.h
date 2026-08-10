#pragma once

#include "mruby_utils.h"

namespace binding {

MRB_DATATYPE_DECLARE(Shader);

void InitShaderBinding(mrb_state* mrb);

}  // namespace binding
