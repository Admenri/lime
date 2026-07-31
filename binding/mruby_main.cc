#include "mruby.h"
#include "mruby/compile.h"

namespace binding {}

extern "C" void rgssx_main() {
  mrb_state* core = mrb_open();

  mrb_load_string(core, "print 'Hello World'");

  mrb_close(core);
}
