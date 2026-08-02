#pragma once

#include "mruby_utils.h"

namespace binding {

MRB_DATATYPE_DECLARE(Tone);

void InitToneBinding(mrb_state* mrb);

}  // namespace binding
