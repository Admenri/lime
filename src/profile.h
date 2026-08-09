#pragma once

#include <string>

#include "src/common.h"
#include "src/inirw.h"

namespace lime {

class Config : public Singleton<Config> {
 public:
  Config(std::string inifile);

  int rgss_version = 0;

  std::string scripts = "Data/Scripts.rvdata2";
  std::string title = "lime";
  std::string rtp;
  std::string soundfont;

  int width = 544;
  int height = 416;
  bool vsync = false;
  bool fullscreen = false;

 private:
  IniFile parser_;
};

}  // namespace lime
