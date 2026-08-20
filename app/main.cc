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

#include "src/audio.h"
#include "src/common.h"
#include "src/filesystem.h"
#include "src/graphics.h"
#include "src/input.h"
#include "src/profile.h"

#if defined(_WIN32)
#include "src/platform/win32.h"
#endif  // _WIN32

extern "C" void lime_main();

int main(int argc, char** argv) {
  // App name
  std::string app(argv[0]);
  for (size_t i = 0; i < app.size(); ++i)
    if (app[i] == '\\')
      app[i] = '/';

  // Game directory (where the executable lives)
  std::string base_dir = ".";
  auto last_sep = app.find_last_of('/');
  if (last_sep != std::string::npos) {
    base_dir = app.substr(0, last_sep);
    app = app.substr(last_sep + 1);
  }

  last_sep = app.find_last_of('.');
  if (last_sep != std::string::npos)
    app = app.substr(0, last_sep);
  std::string ini = app + ".ini";

  // File system
  auto io_service = new lime::IOService(argv[0]);
  lime::IOService::Instance(io_service);

  // The game directory is both the search root and the write root
  io_service->SetWritePath(base_dir);
  io_service->AddLoadPath(".", "/");

  // Global config
  auto config = new lime::Config(ini);
  lime::Config::Instance(config);

  // Global renderer
  auto graphics =
      new lime::Graphics(config->width, config->height, config->title,
                         config->vsync, config->fullscreen);
  lime::Graphics::Instance(graphics);

  // Input manager
  auto input = new lime::Input(config->rgss_version);
  lime::Input::Instance(input);

  // Audio manager
  auto audio = new lime::Audio();
  lime::Audio::Instance(audio);

// RTP reading
#if defined(_WIN32)
  auto rtp_key = config->rtp;
  auto rtp_path = platform::win32::GetRTPPath(config->rgss_version, rtp_key);
  if (rtp_path.has_value()) {
    io_service->AddLoadPath(rtp_path.value(), "/");
    raylib::TraceLog(raylib::LOG_INFO, "RTP: %s", rtp_path.value().c_str());
  }
#endif  // _WIN32

  // Main entry
  lime_main();

  // Clean up
  lime::Graphics::Instance(nullptr);
  lime::Audio::Instance(nullptr);
  lime::Input::Instance(nullptr);
  lime::Config::Instance(nullptr);
  lime::IOService::Instance(nullptr);

  return 0;
}
