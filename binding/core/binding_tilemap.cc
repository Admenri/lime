#include "binding_tilemap.h"

#include "binding_bitmap.h"
#include "binding_table.h"
#include "binding_viewport.h"

#include "src/profile.h"
#include "src/tilemap.h"

namespace binding {

const mrb_data_type kTilemapBitmapsDataType = {"TilemapBitmapArray", nullptr};

MRB_FUNC(TilemapBitmaps_Get) {
  auto* self_obj = GetSelfData<lime::Tilemap>(self);
  mrb_int index;
  mrb_get_args(mrb, "i", &index);

  EXC_BEGIN {
    auto result = self_obj->GetBitmap(index);
    if (result)
      return WrapObject(mrb, result.get(), kBitmapDataType);
    return mrb_nil_value();
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(TilemapBitmaps_Set) {
  auto* self_obj = GetSelfData<lime::Tilemap>(self);
  mrb_int index;
  mrb_value bitmap_val;
  mrb_get_args(mrb, "io", &index, &bitmap_val);

  auto bitmap = GetObject<lime::Bitmap>(mrb, bitmap_val, kBitmapDataType);

  EXC_BEGIN {
    self_obj->SetBitmap(index, bitmap);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

// -------------------------------------------------------------------------

// Define mrb data type
MRB_DATATYPE_DEFINE(Tilemap);

MRB_FUNC(Tilemap_initialize) {
  mrb_value viewport_val = mrb_nil_value();
  mrb_get_args(mrb, "|o", &viewport_val);

  auto viewport =
      GetObject<lime::Viewport>(mrb, viewport_val, kViewportDataType);

  lime::RefPtr<lime::Tilemap> obj = nullptr;
  EXC_BEGIN {
    obj = lime::MakeRefCounted<lime::Tilemap>(viewport);
  }
  EXC_END(mrb);

  return SetupSelfData(self, obj.get(), kTilemapDataType);
}

MRB_FUNC(Tilemap_Update) {
  auto* self_obj = GetSelfData<lime::Tilemap>(self);
  EXC_BEGIN {
    self_obj->Update();
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Tilemap_GetBitmaps) {
  auto* self_obj = GetSelfData<lime::Tilemap>(self);

  RClass* klass = mrb_class_get(mrb, kTilemapBitmapsDataType.struct_name);
  RData* data =
      mrb_data_object_alloc(mrb, klass, self_obj, &kTilemapBitmapsDataType);
  mrb_value obj = mrb_obj_value(data);
  SetupSelfData(obj, self_obj, kTilemapBitmapsDataType);

  return obj;
}

BINDING_ATTR_OBJECT_REF(Tilemap,
                        lime::Tilemap,
                        Viewport,
                        lime::Viewport,
                        kViewportDataType);
BINDING_ATTR_BOOL(Tilemap, lime::Tilemap, Visible);
BINDING_ATTR_INT(Tilemap, lime::Tilemap, Z);
BINDING_ATTR_OBJECT_REF(Tilemap,
                        lime::Tilemap,
                        MapData,
                        lime::Table,
                        kTableDataType);
BINDING_ATTR_OBJECT_REF(Tilemap,
                        lime::Tilemap,
                        FlashData,
                        lime::Table,
                        kTableDataType);
BINDING_ATTR_OBJECT_REF(Tilemap,
                        lime::Tilemap,
                        Flags,
                        lime::Table,
                        kTableDataType);
BINDING_ATTR_INT(Tilemap, lime::Tilemap, OX);
BINDING_ATTR_INT(Tilemap, lime::Tilemap, OY);

// Inherited from Dispoable
BINDING_INHERITED_DISPOABLE(Tilemap, lime::Tilemap);

void InitTilemapBinding(mrb_state* mrb) {
  auto klass = DefineClass(mrb, "Tilemap");

  mrb_define_method(mrb, klass, "initialize", Tilemap_initialize,
                    MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "update", Tilemap_Update, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "bitmaps", Tilemap_GetBitmaps, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "viewport", Tilemap_Viewport, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "viewport=", Tilemap_ViewportEqual,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "visible", Tilemap_Visible, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "visible=", Tilemap_VisibleEqual,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "z", Tilemap_Z, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "z=", Tilemap_ZEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "map_data", Tilemap_MapData, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "map_data=", Tilemap_MapDataEqual,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "flash_data", Tilemap_FlashData,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "flash_data=", Tilemap_FlashDataEqual,
                    MRB_ARGS_REQ(1));
  if (lime::Config::Instance()->rgss_version >= 3) {
    mrb_define_method(mrb, klass, "flags", Tilemap_Flags, MRB_ARGS_NONE());
    mrb_define_method(mrb, klass, "flags=", Tilemap_FlagsEqual,
                      MRB_ARGS_REQ(1));
  } else {
    mrb_define_method(mrb, klass, "passages", Tilemap_Flags, MRB_ARGS_NONE());
    mrb_define_method(mrb, klass, "passages=", Tilemap_FlagsEqual,
                      MRB_ARGS_REQ(1));
  }
  mrb_define_method(mrb, klass, "ox", Tilemap_OX, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "ox=", Tilemap_OXEqual, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "oy", Tilemap_OY, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "oy=", Tilemap_OYEqual, MRB_ARGS_REQ(1));
  // Inherited from Dispoable
  mrb_define_method(mrb, klass, "dispose", Tilemap_Dispose, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "disposed?", Tilemap_IsDisposed,
                    MRB_ARGS_NONE());

  auto bitmaps_klass = DefineClass(mrb, kTilemapBitmapsDataType.struct_name);
  mrb_define_method(mrb, bitmaps_klass, "[]", TilemapBitmaps_Get,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, bitmaps_klass, "[]=", TilemapBitmaps_Set,
                    MRB_ARGS_REQ(2));
}

}  // namespace binding
