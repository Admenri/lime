#include "config.h"

namespace rgssx {

Config::Config(std::string inifile) : parser_(inifile) {
  scripts = parser_.Get("Game", "Scripts", scripts);
  title = parser_.Get("Game", "Title", title);
  rtp = parser_.Get("Game", "RTP", rtp);

  width = parser_.GetInt("Window", "Width", width);
  height = parser_.GetInt("Window", "Height", height);
  vsync = parser_.GetBool("Window", "VSync", vsync);
  fullscreen = parser_.GetInt("Window", "Fullscreen", fullscreen);
}

}  // namespace rgssx
