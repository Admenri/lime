
#include "src/bitmap.h"
#include "src/common.h"
#include "src/config.h"
#include "src/graphics.h"
#include "src/input.h"
#include "src/plane.h"
#include "src/sprite.h"
#include "src/window.h"

#if defined(_WIN32)
#include "src/platform/win32.h"
#endif  // _WIN32

extern "C" void rgssx_main();

int main(int argc, char** argv) {
  // App name
  std::string app(argv[0]);
  for (size_t i = 0; i < app.size(); ++i)
    if (app[i] == '\\')
      app[i] = '/';

  auto last_sep = app.find_last_of('/');
  if (last_sep != std::string::npos)
    app = app.substr(last_sep + 1);

  last_sep = app.find_last_of('.');
  if (last_sep != std::string::npos)
    app = app.substr(0, last_sep);
  std::string ini = app + ".ini";

  // Global config
  auto config = new rgssx::Config(ini);
  rgssx::Config::Instance(config);

  // Global renderer
  rgssx::Graphics::Instance(new rgssx::Graphics(config->width, config->height,
                                                config->title, config->vsync,
                                                config->fullscreen));

  // Input manager
  rgssx::Input::Instance(new rgssx::Input(1));

// RTP reading
#if defined(_WIN32)
  auto rtp_key = config->rtp;
  auto rtp_path = platform::win32::GetRTPPath(3, rtp_key);
  if (rtp_path.has_value())
    raylib::TraceLog(raylib::LOG_INFO, "RTP: %s", rtp_path.value().c_str());
#endif  // _WIN32

  // Main entry
  rgssx_main();

  // Clean up
  rgssx::Graphics::Instance(nullptr);
  rgssx::Input::Instance(nullptr);
  rgssx::Config::Instance(nullptr);

  return 0;
}
