#pragma once

#include <string>

#include "src/common.h"
#include "src/inirw.h"

namespace rgssx {

class Config : public Singleton<Config> {
 public:
  Config(std::string inifile);

  int rgss_version = 0;

  std::string scripts = "Data/Scripts.rvdata2";
  std::string title = "RGSSX";
  std::string rtp;

  int width = 544;
  int height = 416;
  bool vsync = false;
  bool fullscreen = false;

 private:
  IniFile parser_;
};

}  // namespace rgssx
