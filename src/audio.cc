#include "audio.h"

namespace rgssx {

Audio::Audio() {}

Audio::~Audio() {}

void Audio::SetupMIDI() {}

void Audio::BGMPlay(std::string filename, int volume, int pitch, float pos) {}

void Audio::BGMStop() {}

void Audio::BGMFade(int time) {}

float Audio::BGMPos() {
  return 0.0f;
}

void Audio::BGSPlay(std::string filename, int volume, int pitch, float pos) {}

void Audio::BGSStop() {}

void Audio::BGSFade(int time) {}

float Audio::BGSPos() {
  return 0.0f;
}

void Audio::MEPlay(std::string filename, int volume, int pitch) {}

void Audio::MEStop() {}

void Audio::MEFade(int time) {}

void Audio::SEPlay(std::string filename, int volume, int pitch) {}

void Audio::SEStop() {}

}  // namespace rgssx
