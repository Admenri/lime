#pragma once

#include "mruby_utils.h"

namespace binding {

MRB_DATATYPE_DECLARE(Sprite);

void InitSpriteBinding(mrb_state* mrb);

}  // namespace binding
