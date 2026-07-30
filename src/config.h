#pragma once

#include <string>

#include "common.h"
#include "inirw.h"

namespace rgssx {

class Config : public Singleton<Config> {
 public:
  Config(std::string inifile);

  std::string scripts;
  std::string title;
  std::string rtp;

  int width = 544;
  int height = 416;
  bool vsync = false;
  bool fullscreen = false;

 private:
  IniFile parser_;
};

}  // namespace rgssx
