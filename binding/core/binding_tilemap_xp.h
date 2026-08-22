#pragma once

#include "mruby_utils.h"

namespace binding {

MRB_DATATYPE_DECLARE(TilemapXP);

void InitTilemapXPBinding(mrb_state* mrb);

}  // namespace binding