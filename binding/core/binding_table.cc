#include "binding_table.h"

#include "src/table.h"

namespace binding {

// Define mrb data type
MRB_DATATYPE_DEFINE(Table);

MRB_FUNC(Table_initialize) {
  mrb_int xsize, ysize = 1, zsize = 1;
  mrb_get_args(mrb, "i|ii", &xsize, &ysize, &zsize);

  rgssx::RefPtr<rgssx::Table> obj = nullptr;
  EXC_BEGIN {
    obj = rgssx::MakeRefCounted<rgssx::Table>(xsize, ysize, zsize);
  }
  EXC_END(mrb);

  SetupSelfData(self, obj.get(), kTableDataType);
  return self;
}

MRB_FUNC(Table_Resize) {
  auto* self_obj = GetSelfData<rgssx::Table>(self);
  mrb_int xsize, ysize = 1, zsize = 1;
  mrb_get_args(mrb, "i|ii", &xsize, &ysize, &zsize);

  EXC_BEGIN {
    self_obj->Resize(xsize, ysize, zsize);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Table_XSize) {
  auto* self_obj = GetSelfData<rgssx::Table>(self);
  EXC_BEGIN {
    return mrb_fixnum_value(self_obj->XSize());
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Table_YSize) {
  auto* self_obj = GetSelfData<rgssx::Table>(self);
  EXC_BEGIN {
    return mrb_fixnum_value(self_obj->YSize());
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Table_ZSize) {
  auto* self_obj = GetSelfData<rgssx::Table>(self);
  EXC_BEGIN {
    return mrb_fixnum_value(self_obj->ZSize());
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Table_Get) {
  auto* self_obj = GetSelfData<rgssx::Table>(self);
  mrb_int x, y = 0, z = 0;
  mrb_get_args(mrb, "i|ii", &x, &y, &z);

  if (x < 0 || x >= self_obj->XSize() || y < 0 || y >= self_obj->YSize() ||
      z < 0 || z >= self_obj->ZSize()) {
    return mrb_nil_value();
  }

  EXC_BEGIN {
    return mrb_fixnum_value(self_obj->Get(x, y, z));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Table_Set) {
  auto* self_obj = GetSelfData<rgssx::Table>(self);

  mrb_value* argv;
  mrb_int argc;
  mrb_get_args(mrb, "*", &argv, &argc);

  mrb_int x, y, z, value;
  if (argc < 2)
    mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");

  switch (argc) {
    default:
    case 2:
      x = mrb_fixnum(argv[0]);
      value = mrb_fixnum(argv[1]);
      break;
    case 3:
      x = mrb_fixnum(argv[0]);
      y = mrb_fixnum(argv[1]);
      value = mrb_fixnum(argv[2]);
      break;
    case 4:
      x = mrb_fixnum(argv[0]);
      y = mrb_fixnum(argv[1]);
      z = mrb_fixnum(argv[2]);
      value = mrb_fixnum(argv[3]);
      break;
  }

  if (x < 0 || x >= self_obj->XSize() || y < 0 || y >= self_obj->YSize() ||
      z < 0 || z >= self_obj->ZSize()) {
    return mrb_nil_value();
  }

  EXC_BEGIN {
    self_obj->Set(static_cast<int16_t>(value), x, y, z);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

// Marshal serialization (instance method _dump) / deserialization (class
// method _load). Per bindgen.md: classes with MARSHAL_DUMP/MARSHAL_LOAD get
// _dump (method) and _load (class method).
MRB_FUNC(Table__dump) {
  auto* self_obj = GetSelfData<rgssx::Table>(self);
  mrb_int limit;
  mrb_get_args(mrb, "i", &limit);

  EXC_BEGIN {
    auto result =
        rgssx::Table::MarshalDump(rgssx::RefPtr<rgssx::Table>(self_obj));
    return mrb_str_new(mrb, result.data(), static_cast<mrb_int>(result.size()));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Table__load) {
  mrb_value data;
  mrb_get_args(mrb, "o", &data);

  rgssx::RefPtr<rgssx::Table> obj = nullptr;
  EXC_BEGIN {
    obj = rgssx::Table::MarshalLoad(MRBStringValue(data));
  }
  EXC_END(mrb);

  return WrapObject(mrb, obj.get(), kTableDataType);
}

void InitTableBinding(mrb_state* mrb) {
  auto klass = DefineClass(mrb, "Table");

  mrb_define_method(mrb, klass, "initialize", Table_initialize, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "resize", Table_Resize, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "xsize", Table_XSize, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "ysize", Table_YSize, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "zsize", Table_ZSize, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "[]", Table_Get, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "[]=", Table_Set, MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "_dump", Table__dump, MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, klass, "_load", Table__load, MRB_ARGS_REQ(1));
}

}  // namespace binding
