#pragma once

#include <span>
#include <unordered_map>

#include "src/common.h"
#include "src/raywarp.h"
#include "src/refptr.h"

namespace lime {

class Bitmap;

class Shader : public RefCounted<Shader> {
 public:
  Shader(std::string vs_code, std::string fs_code);
  ~Shader();

  /*-export.begin-*/
  void SetValueF(std::string uniform,
                 std::span<float> value,
                 int item_count = 1);
  void SetValueI(std::string uniform,
                 std::span<int32_t> value,
                 int item_count = 1);
  void SetValueU(std::string uniform,
                 std::span<uint32_t> value,
                 int item_count = 1);
  void SetValueT(std::string uniform, RefPtr<Bitmap> texture);
  void SetValueM(std::string uniform, float value[16]);
  /*-export.end-*/

 public:
  void BeginEffect();

 private:
  int GetValueLocation(std::string name);

  raylib::Shader shader_ = {};
  std::unordered_map<std::string, int> locations_;
  std::unordered_map<int, RefPtr<Bitmap>> textures_;
};

}  // namespace lime
