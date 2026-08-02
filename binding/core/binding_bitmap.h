#pragma once

#include "mruby_utils.h"

namespace binding {

MRB_DATATYPE_DECLARE(Bitmap);

void InitBitmapBinding(mrb_state* mrb);

}  // namespace binding
