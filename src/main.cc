
#include "bitmap.h"
#include "common.h"
#include "config.h"
#include "graphics.h"
#include "input.h"
#include "plane.h"
#include "sprite.h"
#include "window.h"

#if defined(_WIN32)
#include "platform/win32.h"
#endif  // _WIN32

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
  rgssx::Graphics::Instance()->ResizeScreen(1920, 1080);
  rgssx::Graphics::Instance()->Attr_FrameRate(120);

  {
    auto vp111 = rgssx::MakeRefCounted<rgssx::Viewport>(100, 100, 600, 600);

    auto vp = rgssx::MakeRefCounted<rgssx::Viewport>(vp111, 100, 100, 100, 100);
    // vp->Attr_Visible(false);

    auto sp = rgssx::MakeRefCounted<rgssx::Sprite>(vp);

    auto bmp = rgssx::MakeRefCounted<rgssx::Bitmap>("1.png");
    sp->Attr_Bitmap(bmp);
    // sp->Attr_Angle(45);
    // sp->Attr_OX(150);
    // sp->Attr_OY(150);
    bmp->FillRect(10, 10, 50, 50, new rgssx::Color(255, 255, 255, 255));
    bmp->DrawText(100, 10, 300, 300, "Test String from Raylib");
    bmp->GradientFillRect(10, 200, 200, 50, new rgssx::Color(255, 0, 0, 255),
                          new rgssx::Color(0, 0, 255, 255));

    auto sp2 = rgssx::MakeRefCounted<rgssx::Sprite>();
    sp2->Attr_Bitmap(bmp);
    sp2->Attr_SrcRect(rgssx::MakeRefCounted<rgssx::Rect>(0, 0, 200, 200));
    sp2->Attr_WaveAmp(20);
    // sp2->Attr_Visible(false);

    bmp->SaveFile("out.png");

    auto pl = rgssx::MakeRefCounted<rgssx::Plane>();
    pl->Attr_Z(-100);
    pl->Attr_Bitmap(rgssx::MakeRefCounted<rgssx::Bitmap>("sky.png"));

    auto win = rgssx::MakeRefCounted<rgssx::Window>(300, 300, 300, 300);
    win->Attr_WindowSkin(rgssx::MakeRefCounted<rgssx::Bitmap>("Window.png"));
    win->Attr_Contents(bmp);
    win->Attr_CursorRect(rgssx::MakeRefCounted<rgssx::Rect>(0, 0, 50, 50));
    win->Attr_Pause(true);
    win->Attr_Tone(
        rgssx::MakeRefCounted<rgssx::Tone>(-68.0f, -68.0f, 68.0f, 0.f));

    int i = 0;
    while (!raylib::WindowShouldClose()) {
      if (raylib::IsKeyPressed(raylib::KEY_F3))
        raylib::ToggleFullscreen();

      i += 10;

      win->Attr_Openness(i);

      win->Update();
      sp2->Update();
      // pl->Attr_OX(pl->Attr_OX().value() + 10);
      // pl->Attr_OY(pl->Attr_OY().value() + 10);
      rgssx::Graphics::Instance()->Update();
      rgssx::Input::Instance()->Update();
    }
  }

  // Clean up
  rgssx::Graphics::Instance(nullptr);
  rgssx::Input::Instance(nullptr);
  rgssx::Config::Instance(nullptr);

  return 0;
}
