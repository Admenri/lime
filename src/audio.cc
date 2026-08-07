#include "src/audio.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <utility>
#include <vector>

#include "src/filesystem.h"
#include "src/profile.h"

// tinysf (TinySoundFont + TinyMidiLoader) are header-only libraries compiled
// into this translation unit. TSF_STATIC/TML_STATIC keep every symbol file
// local so no extra build target or link step is required.
#define TSF_STATIC
#define TSF_IMPLEMENTATION
#include "3rdparty/tinysf/tsf.h"
#define TML_STATIC
#define TML_IMPLEMENTATION
#include "3rdparty/tinysf/tml.h"

namespace rgssx {

namespace {

// Safety cap for simultaneously stacked SE instances. Finished instances are
// reclaimed every frame in Update(), so this only triggers on pathological
// bursts and keeps memory bounded.
constexpr size_t kMaxSimultaneousSE = 64;

// Sample rate used when synthesizing MIDI files with tinysf. raylib converts
// the rendered WAV to the audio device rate on playback.
constexpr int kMidiSampleRate = 44100;

// Output gain applied to the synthesized MIDI (dB). Soundfonts are typically
// hot, -10 dB keeps the rendered PCM at a healthy level.
constexpr float kMidiGainDb = -10.0f;

// Upper bound (in milliseconds) on how much of a MIDI file is rendered. Real
// game tracks are a few minutes; this only guards against broken files.
constexpr double kMaxMidiDurationMs = 10.0 * 60.0 * 1000.0;

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

  if (midi_font_)
    tsf_close(static_cast<tsf*>(midi_font_));
  midi_font_ = nullptr;

  if (raylib::IsAudioDeviceReady())
    raylib::CloseAudioDevice();
}

void Audio::SetupMIDI() {
  if (midi_ready_)
    return;
  midi_ready_ = true;  // only attempt once (config does not change at runtime)

  const std::string& path = Config::Instance()->soundfont;
  if (path.empty()) {
    raylib::TraceLog(raylib::LOG_WARNING,
                     "AUDIO: MIDI disabled, no soundfont configured");
    return;
  }

  // Load the SoundFont through the virtual file system, like every other
  // audio resource (extension resolution and case handling included).
  std::string data;
  try {
    IOService::Instance()->OpenRead(
        path, [&](std::unique_ptr<IOStream> stream, const std::string& ext) {
          data = stream->ReadAll();
          return true;  // matched, stop enumeration
        });
  } catch (const Exception&) {
    raylib::TraceLog(raylib::LOG_WARNING, "AUDIO: Failed to read soundfont: %s",
                     path.c_str());
    return;
  }

  // tsf_load_memory fully decodes the SoundFont into memory, so the input
  // buffer does not need to be kept alive afterwards.
  midi_font_ = tsf_load_memory(data.data(), (int)data.size());
  if (!midi_font_) {
    raylib::TraceLog(raylib::LOG_WARNING, "AUDIO: Failed to parse soundfont: %s",
                     path.c_str());
    return;
  }

  tsf_set_output(static_cast<tsf*>(midi_font_), TSF_STEREO_INTERLEAVED,
                 kMidiSampleRate, kMidiGainDb);
  raylib::TraceLog(raylib::LOG_INFO, "AUDIO: Soundfont loaded: %s",
                   path.c_str());
}

namespace {

// Append a little-endian 16-bit value to a byte buffer.
void AppendU16(std::string& out, uint16_t value) {
  out.push_back(static_cast<char>(value & 0xFF));
  out.push_back(static_cast<char>((value >> 8) & 0xFF));
}

// Append a little-endian 32-bit value to a byte buffer.
void AppendU32(std::string& out, uint32_t value) {
  out.push_back(static_cast<char>(value & 0xFF));
  out.push_back(static_cast<char>((value >> 8) & 0xFF));
  out.push_back(static_cast<char>((value >> 16) & 0xFF));
  out.push_back(static_cast<char>((value >> 24) & 0xFF));
}

// Wrap interleaved float PCM ([-1, 1]) as a 16-bit PCM WAV in memory. raylib
// can then stream it like any other .wav (seek, loop, volume, pitch).
void EncodeWavToMemory(const std::vector<float>& pcm,
                       int sample_rate,
                       std::string* out) {
  const uint32_t channels = 2;
  const uint32_t data_bytes = (uint32_t)(pcm.size() * sizeof(int16_t));

  out->clear();
  out->reserve(44 + data_bytes);

  out->append("RIFF");
  AppendU32(*out, 36 + data_bytes);
  out->append("WAVE");
  out->append("fmt ");
  AppendU32(*out, 16);             // fmt chunk size
  AppendU16(*out, 1);              // PCM
  AppendU16(*out, channels);       // stereo
  AppendU32(*out, sample_rate);    // sample rate
  AppendU32(*out, sample_rate * channels * sizeof(int16_t));  // byte rate
  AppendU16(*out, channels * sizeof(int16_t));                // block align
  AppendU16(*out, sizeof(int16_t) * 8);                       // bits/sample
  out->append("data");
  AppendU32(*out, data_bytes);

  for (float sample : pcm) {
    int16_t value = static_cast<int16_t>(
        std::clamp(static_cast<int>(sample * 32767.5f), -32768, 32767));
    AppendU16(*out, static_cast<uint16_t>(value));
  }
}

}  // namespace

bool Audio::RenderMidiToWav(const std::string& midi_data,
                            std::string* out_wav) {
  if (!midi_font_)
    return false;

  // Parse the MIDI event stream.
  tml_message* messages =
      tml_load_memory(midi_data.data(), (int)midi_data.size());
  if (!messages)
    return false;

  int used_channels = 0, used_programs = 0, total_notes = 0;
  unsigned int time_first_note = 0, time_length = 0;
  tml_get_info(messages, &used_channels, &used_programs, &total_notes,
               &time_first_note, &time_length);
  if (time_length == 0)
    time_length = 1000;  // guard against pathological files

  // Independent playback instance sharing the already loaded SoundFont.
  tsf* player = tsf_copy(static_cast<tsf*>(midi_font_));
  tsf_set_output(player, TSF_STEREO_INTERLEAVED, kMidiSampleRate, kMidiGainDb);

  // Initialize the MIDI percussion channel (10th channel, index 9).
  tsf_channel_set_bank_preset(player, 9, 128, 0);

  // Walk the message list in time order, rendering the PCM in between events.
  std::vector<float> pcm;
  const size_t estimated_frames =
      (size_t)((double)time_length / 1000.0 * kMidiSampleRate) +
      2 * kMidiSampleRate;
  // Pre-allocate typical songs but cap the hint so a pathological file cannot
  // force a huge upfront allocation (the vector still grows as needed).
  pcm.reserve(std::min<size_t>(estimated_frames * 2, 512 * 1024 * 1024));

  auto render_frames = [&](int frames) {
    if (frames <= 0)
      return;
    size_t base = pcm.size();
    pcm.resize(base + (size_t)frames * 2);
    tsf_render_float(player, pcm.data() + base, frames, 0);
  };

  double current_msec = 0.0;
  const tml_message* msg = messages;
  while (msg) {
    double target_msec = (double)msg->time;
    if (target_msec > kMaxMidiDurationMs)
      break;  // pathological file, stop rendering early
    int frames = (int)((target_msec - current_msec) * kMidiSampleRate / 1000.0);
    render_frames(frames);
    current_msec = target_msec;

    switch (msg->type) {
      case TML_PROGRAM_CHANGE:
        tsf_channel_set_presetnumber(player, msg->channel, msg->program,
                                     (msg->channel == 9));
        break;
      case TML_NOTE_ON:
        tsf_channel_note_on(player, msg->channel, msg->key,
                            msg->velocity / 127.0f);
        break;
      case TML_NOTE_OFF:
        tsf_channel_note_off(player, msg->channel, msg->key);
        break;
      case TML_PITCH_BEND:
        tsf_channel_set_pitchwheel(player, msg->channel, msg->pitch_bend);
        break;
      case TML_CONTROL_CHANGE:
        tsf_channel_midi_control(player, msg->channel, msg->control,
                                 msg->control_value);
        break;
      default:
        break;
    }
    msg = msg->next;
  }

  // Render the release tail until every voice has decayed (bounded).
  constexpr int kTailBlockFrames = 64;  // TSF_RENDER_EFFECTSAMPLEBLOCK
  constexpr int kMaxTailFrames = 5 * kMidiSampleRate;
  int tail_frames = 0;
  while (tail_frames < kMaxTailFrames) {
    render_frames(kTailBlockFrames);
    tail_frames += kTailBlockFrames;
    if (tsf_active_voice_count(player) == 0)
      break;
  }

  tml_free(messages);
  tsf_close(player);

  EncodeWavToMemory(pcm, kMidiSampleRate, out_wav);
  return !out_wav->empty();
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

          // MIDI has no raylib decoder: synthesize it to PCM with tinysf and
          // hand the resulting WAV to the regular streaming path.
          bool is_midi = (ext == "mid" || ext == "midi") ||
                         (data.size() >= 4 &&
                          data.compare(0, 4, "MThd") == 0);
          if (is_midi) {
            std::string wav;
            if (RenderMidiToWav(data, &wav)) {
              // The stream decodes lazily from this buffer, so the buffer must
              // outlive the Music object.
              channel->data = std::move(wav);
              channel->music = raylib::LoadMusicStreamFromMemory(
                  ".wav", (const unsigned char*)channel->data.data(),
                  (int)channel->data.size());
              if (raylib::IsMusicValid(channel->music)) {
                channel->loaded = true;
                loaded = true;
              }
            }
            return loaded;  // stop enumeration, MIDI either worked or not
          }

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

          // Same MIDI handling as LoadMusic; the WAV is decoded fully into
          // the Wave, so the temporary buffer can be dropped afterwards.
          bool is_midi = (ext == "mid" || ext == "midi") ||
                         (data.size() >= 4 &&
                          data.compare(0, 4, "MThd") == 0);
          if (is_midi) {
            std::string wav;
            if (RenderMidiToWav(data, &wav)) {
              *wave = raylib::LoadWaveFromMemory(
                  ".wav", (const unsigned char*)wav.data(), (int)wav.size());
              if (raylib::IsWaveValid(*wave))
                loaded = true;
            }
            return loaded;  // stop enumeration
          }

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
