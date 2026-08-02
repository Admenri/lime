#pragma once

#include "mruby_utils.h"

namespace binding {

MRB_DATATYPE_DECLARE(Tilemap);

void InitTilemapBinding(mrb_state* mrb);

}  // namespace binding
