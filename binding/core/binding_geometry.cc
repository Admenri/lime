#include "binding_geometry.h"

#include "binding_bitmap.h"
#include "binding_color.h"
#include "binding_shader.h"
#include "binding_vector.h"
#include "binding_viewport.h"

#include "src/geometry.h"

namespace binding {

// Define mrb data type
MRB_DATATYPE_DEFINE(Geometry);

MRB_FUNC(Geometry_initialize) {
  mrb_int argc = mrb_get_argc(mrb);

  lime::RefPtr<lime::Geometry> obj = nullptr;
  EXC_BEGIN {
    if (argc == 0) {
      // Geometry.new
      obj = lime::MakeRefCounted<lime::Geometry>();
    } else if (argc == 1) {
      // Geometry.new(viewport)
      mrb_value viewport_val;
      mrb_get_args(mrb, "o", &viewport_val);
      obj = lime::MakeRefCounted<lime::Geometry>(
          GetObject<lime::Viewport>(mrb, viewport_val, kViewportDataType));
    } else {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
    }
  }
  EXC_END(mrb);

  return SetupSelfData(self, obj.get(), kGeometryDataType);
}

MRB_FUNC(Geometry_SetPosition) {
  auto* self_obj = GetSelfData<lime::Geometry>(self);
  mrb_int triangle, point;
  mrb_value position_val;
  mrb_get_args(mrb, "iio", &triangle, &point, &position_val);

  auto position =
      GetObject<lime::Vector3>(mrb, position_val, kVector3DataType);

  EXC_BEGIN {
    self_obj->SetPosition(triangle, point, position);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Geometry_SetTexcoord) {
  auto* self_obj = GetSelfData<lime::Geometry>(self);
  mrb_int triangle, point;
  mrb_value texcoord_val;
  mrb_get_args(mrb, "iio", &triangle, &point, &texcoord_val);

  auto texcoord =
      GetObject<lime::Vector2>(mrb, texcoord_val, kVector2DataType);

  EXC_BEGIN {
    self_obj->SetTexcoord(triangle, point, texcoord);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Geometry_SetColor) {
  auto* self_obj = GetSelfData<lime::Geometry>(self);
  mrb_int triangle, point;
  mrb_value color_val;
  mrb_get_args(mrb, "iio", &triangle, &point, &color_val);

  auto color = GetObject<lime::Color>(mrb, color_val, kColorDataType);

  EXC_BEGIN {
    self_obj->SetColor(triangle, point, color);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

BINDING_ATTR_INT(Geometry, lime::Geometry, Capacity);
BINDING_ATTR_OBJECT_REF(Geometry,
                        lime::Geometry,
                        Bitmap,
                        lime::Bitmap,
                        kBitmapDataType);
BINDING_ATTR_INT(Geometry, lime::Geometry, BlendType);
BINDING_ATTR_OBJECT_REF(Geometry,
                        lime::Geometry,
                        Shader,
                        lime::Shader,
                        kShaderDataType);

// Inherited from Dispoable / ViewportChild / Drawable
BINDING_INHERITED_DISPOABLE(Geometry, lime::Geometry);
BINDING_ATTR_OBJECT_REF(Geometry,
                        lime::Geometry,
                        Viewport,
                        lime::Viewport,
                        kViewportDataType);
BINDING_ATTR_BOOL(Geometry, lime::Geometry, Visible);
BINDING_ATTR_INT(Geometry, lime::Geometry, Z);

void InitGeometryBinding(mrb_state* mrb) {
  auto klass = DefineClass(mrb, "Geometry");

  mrb_define_method(mrb, klass, "initialize", Geometry_initialize,
                    MRB_ARGS_ANY());
  mrb_define_method(mrb, klass, "set_position", Geometry_SetPosition,
                    MRB_ARGS_REQ(3));
  mrb_define_method(mrb, klass, "set_texcoord", Geometry_SetTexcoord,
                    MRB_ARGS_REQ(3));
  mrb_define_method(mrb, klass, "set_color", Geometry_SetColor,
                    MRB_ARGS_REQ(3));
  mrb_define_method(mrb, klass, "capacity", Geometry_Capacity,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "capacity=", Geometry_CapacityEqual,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "bitmap", Geometry_Bitmap, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "bitmap=", Geometry_BitmapEqual,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "blend_type", Geometry_BlendType,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "blend_type=", Geometry_BlendTypeEqual,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "shader", Geometry_Shader, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "shader=", Geometry_ShaderEqual,
                    MRB_ARGS_REQ(1));
  // Inherited from Dispoable
  mrb_define_method(mrb, klass, "dispose", Geometry_Dispose, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "disposed?", Geometry_IsDisposed,
                    MRB_ARGS_NONE());
  // Inherited from ViewportChild / Drawable
  mrb_define_method(mrb, klass, "viewport", Geometry_Viewport, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "viewport=", Geometry_ViewportEqual,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "visible", Geometry_Visible, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "visible=", Geometry_VisibleEqual,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, klass, "z", Geometry_Z, MRB_ARGS_NONE());
  mrb_define_method(mrb, klass, "z=", Geometry_ZEqual, MRB_ARGS_REQ(1));
}

}  // namespace binding
