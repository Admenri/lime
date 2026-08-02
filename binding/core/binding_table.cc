#include "binding_table.h"

#include "table.h"

namespace binding {

// Define mrb data type
MRB_DATATYPE_DEFINE(Table);

MRB_FUNC(Table_initialize) {
  mrb_int xsize, ysize = 1, zsize = 1;
  mrb_get_args(mrb, "i|ii", &xsize, &ysize, &zsize);

  rgssx::RefPtr<rgssx::Table> obj = nullptr;
  EXC_BEGIN {
    obj = rgssx::MakeRefCounted<rgssx::Table>(xsize, ysize, zsize);
  } EXC_END(mrb);

  SetupSelfData(self, obj.get(), kTableDataType);
  return self;
}

MRB_FUNC(Table_Resize) {
  auto* self_obj = GetSelfData<rgssx::Table>(self);
  mrb_int xsize, ysize = 1, zsize = 1;
  mrb_get_args(mrb, "i|ii", &xsize, &ysize, &zsize);

  EXC_BEGIN {
    self_obj->Resize(xsize, ysize, zsize);
  } EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Table_XSize) {
  auto* self_obj = GetSelfData<rgssx::Table>(self);
  EXC_BEGIN {
    return mrb_fixnum_value(self_obj->XSize());
  } EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Table_YSize) {
  auto* self_obj = GetSelfData<rgssx::Table>(self);
  EXC_BEGIN {
    return mrb_fixnum_value(self_obj->YSize());
  } EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Table_ZSize) {
  auto* self_obj = GetSelfData<rgssx::Table>(self);
  EXC_BEGIN {
    return mrb_fixnum_value(self_obj->ZSize());
  } EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Table_Get) {
  auto* self_obj = GetSelfData<rgssx::Table>(self);
  mrb_int x, y = 0, z = 0;
  mrb_get_args(mrb, "i|ii", &x, &y, &z);

  EXC_BEGIN {
    return mrb_fixnum_value(self_obj->Get(x, y, z));
  } EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Table_Set) {
  auto* self_obj = GetSelfData<rgssx::Table>(self);
  mrb_int value, x, y = 0, z = 0;
  mrb_get_args(mrb, "ii|ii", &value, &x, &y, &z);

  EXC_BEGIN {
    self_obj->Set(static_cast<int16_t>(value), x, y, z);
  } EXC_END(mrb);
  return mrb_nil_value();
}

void InitTableBinding(mrb_state* mrb) {
  auto klass = DefineClass(mrb, "Table");

  mrb_define_method(mrb, klass, "initialize", Table_initialize, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "resize", Table_Resize, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "x_size", Table_XSize, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "y_size", Table_YSize, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "z_size", Table_ZSize, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "[]", Table_Get, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "[]=", Table_Set, MRB_ARGS_ANY());
}

}  // namespace binding
