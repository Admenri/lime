#pragma once

#include "mruby_utils.h"

namespace binding {

MRB_DATATYPE_DECLARE(Palette);

void InitPaletteBinding(mrb_state* mrb);

}  // namespace binding
