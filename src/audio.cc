#include "src/audio.h"

#include <algorithm>
#include <cctype>
#include <utility>

#include "src/filesystem.h"

namespace rgssx {

namespace {

// Safety cap for simultaneously stacked SE instances. Finished instances are
// reclaimed every frame in Update(), so this only triggers on pathological
// bursts and keeps memory bounded.
constexpr size_t kMaxSimultaneousSE = 64;

}  // namespace

Audio::Audio() {
  raylib::InitAudioDevice();
}

Audio::~Audio() {
  SEStop();
  for (auto& [name, wave] : se_waves_)
    raylib::UnloadWave(wave);
  se_waves_.clear();

  UnloadMusicChannel(&bgm_);
  UnloadMusicChannel(&bgs_);
  UnloadMusicChannel(&me_);

  if (raylib::IsAudioDeviceReady())
    raylib::CloseAudioDevice();
}

void Audio::SetupMIDI() {
  // raylib has no MIDI decoder, nothing to configure.
}

// ---------------------------------------------------------------- helpers

std::string Audio::NormalizeName(const std::string& filename) {
  std::string name = filename;
  for (auto& c : name)
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

  // Strip a trailing extension so "Theme1" and "Theme1.ogg" compare equal.
  auto dot = name.find_last_of('.');
  auto slash = name.find_last_of('/');
  if (dot != std::string::npos && (slash == std::string::npos || dot > slash))
    name = name.substr(0, dot);

  return name;
}

float Audio::VolumeToFloat(int volume) {
  return std::clamp(volume, 0, 100) / 100.0f;
}

float Audio::PitchToFloat(int pitch) {
  return std::clamp(pitch, 50, 150) / 100.0f;
}

bool Audio::LoadMusic(MusicChannel* channel, const std::string& filename) {
  bool loaded = false;
  try {
    IOService::Instance()->OpenRead(
        filename,
        [&](std::unique_ptr<IOStream> stream, const std::string& ext) {
          std::string data = stream->ReadAll();
          std::string file_type = "." + ext;
          raylib::Music music = raylib::LoadMusicStreamFromMemory(
              file_type.c_str(), (const unsigned char*)data.data(),
              (int)data.size());
          if (raylib::IsMusicValid(music)) {
            // The stream decodes lazily from this buffer, so the buffer must
            // outlive the Music object.
            channel->data = std::move(data);
            channel->music = music;
            channel->loaded = true;
            loaded = true;
          }
          return loaded;  // true -> stop enumeration
        });
  } catch (const Exception&) {
    return false;
  }
  return loaded;
}

bool Audio::LoadWave(raylib::Wave* wave, const std::string& filename) {
  bool loaded = false;
  try {
    IOService::Instance()->OpenRead(
        filename,
        [&](std::unique_ptr<IOStream> stream, const std::string& ext) {
          std::string data = stream->ReadAll();
          std::string file_type = "." + ext;
          *wave = raylib::LoadWaveFromMemory(file_type.c_str(),
                                             (const unsigned char*)data.data(),
                                             (int)data.size());
          if (raylib::IsWaveValid(*wave))
            loaded = true;
          return loaded;  // true -> stop enumeration
        });
  } catch (const Exception&) {
    return false;
  }
  return loaded;
}

void Audio::UnloadMusicChannel(MusicChannel* channel) {
  if (channel->loaded)
    raylib::UnloadMusicStream(channel->music);
  channel->music = {};
  channel->data.clear();
  channel->name.clear();
  channel->loaded = false;
  channel->active = false;
}

void Audio::StartMusic(MusicChannel* channel,
                       FadeState* fade,
                       const std::string& filename,
                       int volume,
                       int pitch,
                       float pos,
                       bool looping,
                       bool suspend_for_me) {
  std::string name = NormalizeName(filename);

  // The same track is already selected: only retune (and seek when a position
  // is requested), never restart it from the beginning.
  if (channel->loaded && channel->name == name) {
    channel->volume = VolumeToFloat(volume);
    channel->pitch = PitchToFloat(pitch);
    fade->active = false;
    raylib::SetMusicVolume(channel->music, channel->volume);
    raylib::SetMusicPitch(channel->music, channel->pitch);
    if (pos > 0.0f)
      raylib::SeekMusicStream(channel->music, pos);

    if (!channel->active) {
      // It was stopped earlier, restart it now.
      raylib::PlayMusicStream(channel->music);
      raylib::UpdateMusicStream(channel->music);
      channel->active = true;
      if (suspend_for_me && bgm_paused_for_me_)
        raylib::PauseMusicStream(channel->music);
    }
    return;
  }

  // A different track: replace the current one.
  if (channel->loaded)
    UnloadMusicChannel(channel);

  fade->active = false;
  if (!LoadMusic(channel, filename)) {
    raylib::TraceLog(raylib::LOG_WARNING, "AUDIO: Failed to load music: %s",
                     filename.c_str());
    return;
  }

  channel->name = name;
  channel->volume = VolumeToFloat(volume);
  channel->pitch = PitchToFloat(pitch);
  channel->music.looping = looping;
  channel->active = true;

  raylib::PlayMusicStream(channel->music);
  raylib::UpdateMusicStream(channel->music);
  raylib::SetMusicVolume(channel->music, channel->volume);
  raylib::SetMusicPitch(channel->music, channel->pitch);
  if (pos > 0.0f)
    raylib::SeekMusicStream(channel->music, pos);

  // If an ME is playing, the BGM must stay muted until the ME ends.
  if (suspend_for_me && bgm_paused_for_me_)
    raylib::PauseMusicStream(channel->music);
}

void Audio::StartME(const std::string& filename, int volume, int pitch) {
  if (me_.loaded)
    UnloadMusicChannel(&me_);
  me_fade_.active = false;

  if (!LoadMusic(&me_, filename)) {
    raylib::TraceLog(raylib::LOG_WARNING, "AUDIO: Failed to load ME: %s",
                     filename.c_str());
    ResumeBGM();
    return;
  }

  me_.name = NormalizeName(filename);
  me_.volume = VolumeToFloat(volume);
  me_.pitch = PitchToFloat(pitch);
  me_.music.looping = false;
  me_.active = true;

  // Suspend the BGM while the ME is playing.
  if (bgm_.loaded && bgm_.active && !bgm_paused_for_me_)
    raylib::PauseMusicStream(bgm_.music);
  bgm_paused_for_me_ = true;

  raylib::PlayMusicStream(me_.music);
  raylib::UpdateMusicStream(me_.music);
  raylib::SetMusicVolume(me_.music, me_.volume);
  raylib::SetMusicPitch(me_.music, me_.pitch);
}

void Audio::StopMEChannel() {
  if (me_.loaded)
    UnloadMusicChannel(&me_);
  ResumeBGM();
}

void Audio::ResumeBGM() {
  if (!bgm_paused_for_me_)
    return;
  bgm_paused_for_me_ = false;
  if (bgm_.loaded && bgm_.active)
    raylib::ResumeMusicStream(bgm_.music);
}

bool Audio::UpdateFade(MusicChannel* channel, FadeState* fade, float dt) {
  if (!fade->active)
    return false;

  fade->elapsed += dt;
  float t = std::min(1.0f, fade->elapsed / fade->duration);
  if (channel->loaded)
    raylib::SetMusicVolume(channel->music, fade->start_volume * (1.0f - t));

  if (t >= 1.0f) {
    fade->active = false;
    if (channel->loaded)
      raylib::StopMusicStream(channel->music);
    channel->active = false;
    return true;
  }
  return false;
}

// ---------------------------------------------------------------- BGM

void Audio::BGMPlay(std::string filename, int volume, int pitch, float pos) {
  if (!raylib::IsAudioDeviceReady())
    return;
  StartMusic(&bgm_, &bgm_fade_, filename, volume, pitch, pos,
             /*looping=*/true, /*suspend_for_me=*/true);
}

void Audio::BGMStop() {
  if (bgm_.loaded)
    raylib::StopMusicStream(bgm_.music);
  bgm_.active = false;
  bgm_fade_.active = false;
}

void Audio::BGMFade(int time) {
  if (!bgm_.loaded || !bgm_.active)
    return;
  bgm_fade_.active = true;
  bgm_fade_.start_volume = bgm_.volume;
  bgm_fade_.elapsed = 0.0f;
  bgm_fade_.duration = std::max(1, time) / 1000.0f;
}

float Audio::BGMPos() {
  if (!bgm_.loaded || !bgm_.active)
    return 0.0f;
  return raylib::GetMusicTimePlayed(bgm_.music);
}

// ---------------------------------------------------------------- BGS

void Audio::BGSPlay(std::string filename, int volume, int pitch, float pos) {
  if (!raylib::IsAudioDeviceReady())
    return;
  StartMusic(&bgs_, &bgs_fade_, filename, volume, pitch, pos,
             /*looping=*/true, /*suspend_for_me=*/false);
}

void Audio::BGSStop() {
  if (bgs_.loaded)
    raylib::StopMusicStream(bgs_.music);
  bgs_.active = false;
  bgs_fade_.active = false;
}

void Audio::BGSFade(int time) {
  if (!bgs_.loaded || !bgs_.active)
    return;
  bgs_fade_.active = true;
  bgs_fade_.start_volume = bgs_.volume;
  bgs_fade_.elapsed = 0.0f;
  bgs_fade_.duration = std::max(1, time) / 1000.0f;
}

float Audio::BGSPos() {
  if (!bgs_.loaded || !bgs_.active)
    return 0.0f;
  return raylib::GetMusicTimePlayed(bgs_.music);
}

// ---------------------------------------------------------------- ME

void Audio::MEPlay(std::string filename, int volume, int pitch) {
  if (!raylib::IsAudioDeviceReady())
    return;
  StartME(filename, volume, pitch);
}

void Audio::MEStop() {
  StopMEChannel();
}

void Audio::MEFade(int time) {
  if (!me_.loaded || !me_.active)
    return;
  me_fade_.active = true;
  me_fade_.start_volume = me_.volume;
  me_fade_.elapsed = 0.0f;
  me_fade_.duration = std::max(1, time) / 1000.0f;
}

// ---------------------------------------------------------------- SE

void Audio::SEPlay(std::string filename, int volume, int pitch) {
  if (!raylib::IsAudioDeviceReady())
    return;

  std::string name = NormalizeName(filename);

  // Reuse the decoded wave when the same file is played again.
  raylib::Wave wave = {};
  bool found = false;
  for (auto& [wave_name, cached] : se_waves_) {
    if (wave_name == name) {
      wave = cached;
      found = true;
      break;
    }
  }
  if (!found) {
    if (!LoadWave(&wave, filename))
      return;
    se_waves_.push_back({name, wave});
  }

  // Every SEPlay() starts an independent instance that plays exactly once,
  // even when the same file is played several times in a row.
  raylib::Sound sound = raylib::LoadSoundFromWave(wave);
  if (!raylib::IsSoundValid(sound))
    return;
  raylib::SetSoundVolume(sound, VolumeToFloat(volume));
  raylib::SetSoundPitch(sound, PitchToFloat(pitch));
  raylib::PlaySound(sound);

  // Bound the number of simultaneous instances.
  if (se_tracks_.size() >= kMaxSimultaneousSE) {
    auto& oldest = se_tracks_.front();
    if (raylib::IsSoundPlaying(oldest.sound))
      raylib::StopSound(oldest.sound);
    raylib::UnloadSound(oldest.sound);
    se_tracks_.erase(se_tracks_.begin());
  }

  SoundTrack track;
  track.sound = sound;
  track.age = 0.0f;
  se_tracks_.push_back(track);
}

void Audio::SEStop() {
  for (auto& track : se_tracks_) {
    raylib::StopSound(track.sound);
    raylib::UnloadSound(track.sound);
  }
  se_tracks_.clear();
}

// ---------------------------------------------------------------- update

void Audio::Update() {
  if (!raylib::IsAudioDeviceReady())
    return;

  float dt = raylib::GetFrameTime();

  // Keep the streaming buffers of the active music filled.
  if (bgm_.loaded && bgm_.active)
    raylib::UpdateMusicStream(bgm_.music);
  if (bgs_.loaded && bgs_.active)
    raylib::UpdateMusicStream(bgs_.music);
  if (me_.loaded && me_.active)
    raylib::UpdateMusicStream(me_.music);

  // Progress the volume fades.
  UpdateFade(&bgm_, &bgm_fade_, dt);
  UpdateFade(&bgs_, &bgs_fade_, dt);
  if (UpdateFade(&me_, &me_fade_, dt))
    ResumeBGM();  // ME faded out: bring the BGM back

  // ME finished playing on its own: restore the BGM.
  if (me_.loaded && me_.active && !raylib::IsMusicStreamPlaying(me_.music))
    StopMEChannel();

  // Drop SE instances that already finished playing.
  for (auto it = se_tracks_.begin(); it != se_tracks_.end();) {
    it->age += dt;
    if (it->age > 0.05f && !raylib::IsSoundPlaying(it->sound)) {
      raylib::UnloadSound(it->sound);
      it = se_tracks_.erase(it);
    } else {
      ++it;
    }
  }
}

}  // namespace rgssx
