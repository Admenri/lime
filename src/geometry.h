#pragma once

#include "src/common.h"
#include "src/raywarp.h"
#include "src/refptr.h"
#include "src/viewport.h"

namespace lime {

class Geometry : public RefCounted<Geometry>,
                 public Dispoable,
                 public ViewportChild {
 public:
  Geometry(RefPtr<Viewport> viewport = nullptr);
  ~Geometry();

  /*-export.begin-*/
  void SetPosition(int triangle, int point, float x, float y, float z);
  void SetTexcoord(int triangle, int point, float x, float y);
  void SetColor(int triangle, int point, RefPtr<Color> color);

  ATTR(int, Capacity);
  ATTR(RefPtr<Bitmap>, Bitmap);
  ATTR(int, BlendType);
  ATTR(RefPtr<Shader>, Shader);
  /*-export.end-*/

 private:
  void DisposeObject() override;
  void Draw(DrawParam param) override;

  struct TriangleData {
    raylib::Vector3 position[3] = {};
    raylib::Vector2 texcoord[3] = {};
    raylib::Color color[3] = {};
  };

  RefPtr<Bitmap> bitmap_;
  int blend_type_ = 0;
  RefPtr<Shader> shader_;

  std::vector<TriangleData> data_;
};

}  // namespace lime
