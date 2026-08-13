#pragma once

#include "mruby_utils.h"

namespace binding {

MRB_DATATYPE_DECLARE(Effect);

void InitEffectBinding(mrb_state* mrb);

}  // namespace binding
