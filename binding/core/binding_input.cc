#include "binding_input.h"

#include "input.h"

namespace binding {

MRB_FUNC(Input_Update) {
  auto* self_obj = rgssx::Input::Instance();
  EXC_BEGIN {
    self_obj->Update();
  } EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Input_Pressed) {
  auto* self_obj = rgssx::Input::Instance();
  mrb_sym sym;
  mrb_get_args(mrb, "n", &sym);
  const char* name = mrb_sym2name(mrb, sym);

  EXC_BEGIN {
    return mrb_bool_value(self_obj->Pressed(name));
  } EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Input_Triggered) {
  auto* self_obj = rgssx::Input::Instance();
  mrb_sym sym;
  mrb_get_args(mrb, "n", &sym);
  const char* name = mrb_sym2name(mrb, sym);

  EXC_BEGIN {
    return mrb_bool_value(self_obj->Triggered(name));
  } EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Input_Repeated) {
  auto* self_obj = rgssx::Input::Instance();
  mrb_sym sym;
  mrb_get_args(mrb, "n", &sym);
  const char* name = mrb_sym2name(mrb, sym);

  EXC_BEGIN {
    return mrb_bool_value(self_obj->Repeated(name));
  } EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Input_Dir4) {
  auto* self_obj = rgssx::Input::Instance();
  EXC_BEGIN {
    return mrb_fixnum_value(self_obj->Dir4());
  } EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Input_Dir8) {
  auto* self_obj = rgssx::Input::Instance();
  EXC_BEGIN {
    return mrb_fixnum_value(self_obj->Dir8());
  } EXC_END(mrb);
  return mrb_nil_value();
}

void InitInputBinding(mrb_state* mrb) {
  auto mod = mrb_define_module(mrb, "Input");

  mrb_define_module_function(mrb, mod, "update", Input_Update, MRB_ARGS_NONE());
  mrb_define_module_function(mrb, mod, "press?", Input_Pressed, MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, mod, "trigger?", Input_Triggered, MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, mod, "repeat?", Input_Repeated, MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, mod, "dir4", Input_Dir4, MRB_ARGS_NONE());
  mrb_define_module_function(mrb, mod, "dir8", Input_Dir8, MRB_ARGS_NONE());
}

}  // namespace binding
