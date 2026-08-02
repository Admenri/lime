
#include "src/common.h"
#include "src/config.h"
#include "src/filesystem.h"
#include "src/graphics.h"
#include "src/input.h"

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
  auto io_service = new rgssx::IOService(argv[0]);
  rgssx::IOService::Instance(io_service);

  // The game directory is both the search root and the write root
  io_service->SetWritePath(base_dir);
  io_service->AddLoadPath(base_dir, "/");

  // Global config
  auto config = new rgssx::Config(ini);
  rgssx::Config::Instance(config);

  // Global renderer
  auto graphics =
      new rgssx::Graphics(config->width, config->height, config->title,
                          config->vsync, config->fullscreen);
  rgssx::Graphics::Instance(graphics);

  // Input manager
  auto input = new rgssx::Input(config->rgss_version);
  rgssx::Input::Instance(input);

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
  rgssx_main();

  // Clean up
  rgssx::Graphics::Instance(nullptr);
  rgssx::Input::Instance(nullptr);
  rgssx::Config::Instance(nullptr);
  rgssx::IOService::Instance(nullptr);

  return 0;
}
