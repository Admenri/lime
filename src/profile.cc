#include "src/profile.h"

#include "src/filesystem.h"

namespace lime {

static void ReplaceStringWidth(std::string& str, char before, char after) {
  for (size_t i = 0; i < str.size(); ++i)
    if (str[i] == before)
      str[i] = after;
}

Config::Config(std::string inifile) {
  // Load the ini through the virtual file system (explicit extension)
  try {
    auto stream = IOService::Instance()->OpenReadRaw(inifile);
    auto content = stream->ReadAll();
    parser_.LoadFromString(content);
  } catch (const Exception&) {
    // Missing or unreadable ini: fall back to defaults
  }

  rgss_version = parser_.GetInt("Game", "RGSS", rgss_version);

  scripts = parser_.Get("Game", "Scripts", scripts);
  ReplaceStringWidth(scripts, '\\', '/');
  title = parser_.Get("Game", "Title", title);
  rtp = parser_.Get("Game", "RTP", rtp);

  // Auto-detect the RGSS version from the scripts file extension when
  // it is not explicitly configured (rgss_version == 0).
  if (rgss_version == 0) {
    std::string ext;
    auto dot = scripts.find_last_of('.');
    if (dot != std::string::npos)
      ext = scripts.substr(dot);
    else
      ext = scripts;

    if (ext == ".rxdata")
      rgss_version = 1;
    else if (ext == ".rvdata")
      rgss_version = 2;
    else if (ext == ".rvdata2")
      rgss_version = 3;
  }

  soundfont = parser_.Get("Audio", "Soundfont", soundfont);

  width = parser_.GetInt("Window", "Width", width);
  height = parser_.GetInt("Window", "Height", height);
  vsync = parser_.GetBool("Window", "VSync", vsync);
  fullscreen = parser_.GetInt("Window", "Fullscreen", fullscreen);
}

}  // namespace lime
