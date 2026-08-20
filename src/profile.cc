// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Admenri Adev <admenri0504@gmail.com>.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the “Software”), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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
