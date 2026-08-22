#include "binding_tilemap_xp.h"

#include "binding_bitmap.h"
#include "binding_table.h"
#include "binding_viewport.h"

#include "src/tilemap_xp.h"

namespace binding {

const mrb_data_type kTilemapAutotileDataType = {"TilemapAutotile", nullptr};

MRB_FUNC(TilemapAutotile_Get) {
  auto* self_obj = GetSelfData<lime::TilemapXP>(self);
  mrb_int index;
  mrb_get_args(mrb, "i", &index);

  EXC_BEGIN {
    auto result = self_obj->GetAutotile(index);
    if (result)
      return WrapObject(mrb, result.get(), kBitmapDataType);
    return mrb_nil_value();
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(TilemapAutotile_Set) {
  auto* self_obj = GetSelfData<lime::TilemapXP>(self);
  mrb_int index;
  mrb_value bitmap_val;
  mrb_get_args(mrb, "io", &index, &bitmap_val);

  auto bitmap = GetObject<lime::Bitmap>(mrb, bitmap_val, kBitmapDataType);

  EXC_BEGIN {
    self_obj->SetAutotile(index, bitmap);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

// -------------------------------------------------------------------------

MRB_DATATYPE_DEFINE(TilemapXP);

MRB_FUNC(TilemapXP_initialize) {
  mrb_value viewport_val = mrb_nil_value();
  mrb_get_args(mrb, "|o", &viewport_val);
  auto viewport =
      GetObject<lime::Viewport>(mrb, viewport_val, kViewportDataType);

  lime::RefPtr<lime::TilemapXP> obj = nullptr;
  EXC_BEGIN {
    obj = lime::MakeRefCounted<lime::TilemapXP>(viewport);
  }
  EXC_END(mrb);
  return SetupSelfData(self, obj.get(), kTilemapXPDataType);
}

MRB_FUNC(TilemapXP_Update) {
  auto* self_obj = GetSelfData<lime::TilemapXP>(self);
  EXC_BEGIN {
    self_obj->Update();
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(TilemapXP_SetTileset) {
  auto* self_obj = GetSelfData<lime::TilemapXP>(self);
  mrb_value bitmap_val;
  mrb_get_args(mrb, "o", &bitmap_val);
  auto bitmap = GetObject<lime::Bitmap>(mrb, bitmap_val, kBitmapDataType);
  EXC_BEGIN {
    self_obj->SetTileset(bitmap);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(TilemapXP_GetTileset) {
  auto* self_obj = GetSelfData<lime::TilemapXP>(self);
  EXC_BEGIN {
    auto result = self_obj->GetTileset();
    return WrapObject(mrb, result.get(), kBitmapDataType);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(TilemapXP_Autotiles) {
  auto* self_obj = GetSelfData<lime::TilemapXP>(self);

  RClass* klass = mrb_class_get(mrb, kTilemapAutotileDataType.struct_name);
  RData* data =
      mrb_data_object_alloc(mrb, klass, self_obj, &kTilemapAutotileDataType);
  mrb_value obj = mrb_obj_value(data);
  SetupSelfData(obj, self_obj, kTilemapAutotileDataType);

  return obj;
}

BINDING_ATTR_OBJECT_REF(TilemapXP,
                        lime::TilemapXP,
                        Viewport,
                        lime::Viewport,
                        kViewportDataType);
BINDING_ATTR_BOOL(TilemapXP, lime::TilemapXP, Visible);
BINDING_ATTR_INT(TilemapXP, lime::TilemapXP, Z);
BINDING_ATTR_OBJECT_REF(TilemapXP,
                        lime::TilemapXP,
                        MapData,
                        lime::Table,
                        kTableDataType);
BINDING_ATTR_OBJECT_REF(TilemapXP,
                        lime::TilemapXP,
                        FlashData,
                        lime::Table,
                        kTableDataType);
BINDING_ATTR_OBJECT_REF(TilemapXP,
                        lime::TilemapXP,
                        Priorities,
                        lime::Table,
                        kTableDataType);
BINDING_ATTR_INT(TilemapXP, lime::TilemapXP, OX);
BINDING_ATTR_INT(TilemapXP, lime::TilemapXP, OY);

BINDING_INHERITED_DISPOABLE(TilemapXP, lime::TilemapXP);

void InitTilemapXPBinding(mrb_state* mrb) {
  auto klass = DefineClass(mrb, "TilemapXP");
  mrb_define_method(mrb, klass, "initialize", TilemapXP_initialize,
                    MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "update", TilemapXP_Update, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "tileset=", TilemapXP_SetTileset,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "tileset", TilemapXP_GetTileset,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "autotiles", TilemapXP_Autotiles,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "viewport", TilemapXP_Viewport,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "viewport=", TilemapXP_ViewportEqual,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "visible", TilemapXP_Visible, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "visible=", TilemapXP_VisibleEqual,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "z", TilemapXP_Z, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "z=", TilemapXP_ZEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "map_data", TilemapXP_MapData, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "map_data=", TilemapXP_MapDataEqual,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "flash_data", TilemapXP_FlashData,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "flash_data=", TilemapXP_FlashDataEqual,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "priorities", TilemapXP_Priorities,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "priorities=", TilemapXP_PrioritiesEqual,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "ox", TilemapXP_OX, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "ox=", TilemapXP_OXEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "oy", TilemapXP_OY, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "oy=", TilemapXP_OYEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "dispose", TilemapXP_Dispose, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "disposed?", TilemapXP_IsDisposed,
                    MRB_ARGS_NONE());

  auto bitmaps_klass = DefineClass(mrb, kTilemapAutotileDataType.struct_name);
  mrb_define_method(mrb, bitmaps_klass, "[]", TilemapAutotile_Get,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, bitmaps_klass, "[]=", TilemapAutotile_Set,
                    MRB_ARGS_REQ(2));
}

}  // namespace binding