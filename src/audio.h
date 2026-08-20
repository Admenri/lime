// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Admenri Adev <admenri0504@gmail.com>.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <string>
#include <utility>
#include <vector>

#include "src/common.h"
#include "src/raywarp.h"

namespace lime {

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

 public:
  // Called once per frame by the application. Drives streaming, fades and the
  // ME -> BGM handoff. Never spawns threads, so it is emscripten-safe.
  void Update();

 private:
  // A streamed music track together with the raw file data it references.
  // raylib decodes the stream lazily from that buffer, so the buffer must
  // outlive the Music object.
  struct MusicChannel {
    raylib::Music music = {};
    std::string data;     // backing file data (must outlive music)
    std::string name;     // normalized name used for same-track detection
    float volume = 1.0f;  // normalized 0..1
    float pitch = 1.0f;   // normalized (100 = 1.0)
    bool loaded = false;  // a valid Music object is held
    bool active = false;  // this is the currently selected track (not stopped)
  };

  // A playing sound effect; every SEPlay() creates a new instance.
  struct SoundTrack {
    raylib::Sound sound = {};
    float age = 0.0f;  // seconds since it was started
  };

  // A volume fade-out state.
  struct FadeState {
    float start_volume = 1.0f;
    float elapsed = 0.0f;
    float duration = 0.0f;  // seconds
    bool active = false;
  };

  MusicChannel bgm_;
  MusicChannel bgs_;
  MusicChannel me_;

  FadeState bgm_fade_;
  FadeState bgs_fade_;
  FadeState me_fade_;

  // The BGM is suspended while an ME is playing and resumes when it ends.
  bool bgm_paused_for_me_ = false;

  // MIDI support (tinysf). The soundfont is loaded once by SetupMIDI().
  // Stored as void* so tinysf's types stay out of the public header.
  void* midi_font_ = nullptr;
  bool midi_ready_ = false;  // SetupMIDI() was already attempted

  std::vector<SoundTrack> se_tracks_;
  std::vector<std::pair<std::string, raylib::Wave>> se_waves_;

  static std::string NormalizeName(const std::string& filename);
  static float VolumeToFloat(int volume);
  static float PitchToFloat(int pitch);

  bool LoadMusic(MusicChannel* channel, const std::string& filename);
  bool LoadWave(raylib::Wave* wave, const std::string& filename);
  void UnloadMusicChannel(MusicChannel* channel);

  // Decode a MIDI stream with tinysf and wrap the rendered PCM in a WAV
  // buffer so it can be played through the regular raylib music path.
  bool RenderMidiToWav(const std::string& midi_data, std::string* out_wav);

  void StartMusic(MusicChannel* channel,
                  FadeState* fade,
                  const std::string& filename,
                  int volume,
                  int pitch,
                  float pos,
                  bool looping,
                  bool suspend_for_me);
  void StartME(const std::string& filename, int volume, int pitch);
  void StopMEChannel();

  void ResumeBGM();
  bool UpdateFade(MusicChannel* channel, FadeState* fade, float dt);
};

}  // namespace lime
