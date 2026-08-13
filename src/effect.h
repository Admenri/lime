#pragma once

#include <span>
#include <unordered_map>

#include "src/common.h"
#include "src/raywarp.h"
#include "src/refptr.h"

namespace lime {

class Bitmap;

class Effect : public RefCounted<Effect> {
 public:
  struct ColorBlendState {
    int src_rgb = RL_ONE;
    int dst_rgb = RL_ONE_MINUS_SRC_ALPHA;
    int src_alpha = RL_ONE;
    int dst_alpha = RL_ONE_MINUS_SRC_ALPHA;
    int equal_rgb = RL_FUNC_ADD;
    int equal_alpha = RL_FUNC_ADD;
  };

  struct EffectCreateInfo {
    std::optional<std::string> vertex_shader;
    std::optional<std::string> fragment_shader;
    std::optional<ColorBlendState> color_blend;
  };

  Effect(const EffectCreateInfo& create_info);
  ~Effect();

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
  std::optional<ColorBlendState> color_blend_;

  std::unordered_map<std::string, int> locations_;
  std::unordered_map<int, RefPtr<Bitmap>> textures_;
};

}  // namespace lime
