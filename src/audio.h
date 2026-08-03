#pragma once

#include "src/common.h"

namespace rgssx {

class Audio : public Singleton<Audio> {
 public:
  Audio();
  ~Audio();

  /*-export.begin-*/
  void SetupMIDI();

  void BGMPlay(std::string filename,
               int volume = 100,
               int pitch = 100,
               float pos = 0.0f);
  void BGMStop();
  void BGMFade(int time);
  float BGMPos();

  void BGSPlay(std::string filename,
               int volume = 100,
               int pitch = 100,
               float pos = 0.0f);
  void BGSStop();
  void BGSFade(int time);
  float BGSPos();

  void MEPlay(std::string filename, int volume = 100, int pitch = 100);
  void MEStop();
  void MEFade(int time);

  void SEPlay(std::string filename, int volume = 100, int pitch = 100);
  void SEStop();
  /*-export.end-*/

 private:
};

}  // namespace rgssx
