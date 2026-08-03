#pragma once

#include "mruby_utils.h"

namespace binding {

// Audio is bound as a Ruby module (module functions access the singleton
// through Audio::Instance()).
void InitAudioBinding(mrb_state* mrb);

}  // namespace binding
