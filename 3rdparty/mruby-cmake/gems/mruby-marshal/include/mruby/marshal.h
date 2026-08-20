#pragma once
#include <mruby.h>

// Marshal.dump / Marshal.load for mruby — port of CRuby's marshal.c (Ruby 4.0.6).
void init_marshal(mrb_state *mrb);
