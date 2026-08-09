
#include <algorithm>

#include "zlib.h"

#include "mruby_utils.h"

#include "mruby/debug.h"
#include "mruby/internal.h"

#include "3rdparty/mruby-cmake/gems/mruby-marshal/include/mruby/marshal.h"

#include "src/filesystem.h"
#include "src/graphics.h"
#include "src/profile.h"

#include "core/binding_init.h"

#include "stdlib/dir.h"

#include "rpg/rpg_rgss1.h"
#include "rpg/rpg_rgss2.h"
#include "rpg/rpg_rgss3.h"

namespace binding {

RClass* g_reset_exception = nullptr;
RClass* g_rgss_exception = nullptr;
RClass* g_exit_exception = nullptr;

static int MarshalReaderString(mrb_state* mrb,
                               mrb_value src,
                               void* dest,
                               int size,
                               mrb_uint position) {
  int remain = RSTRING_LEN(src) - position;
  if (size < 0)
    mrb_raisef(mrb, E_ARGUMENT_ERROR, "negative length %d given", size);

  if (remain > 0) {
    int len = remain < size ? remain : size;
    std::memcpy(dest, RSTRING_PTR(src) + position, len);
    return len;
  }

  return 0;
}

static int MarshalWriterString(mrb_state* mrb,
                               const void* src,
                               int size,
                               mrb_value dest,
                               mrb_uint position) {
  int ai = mrb_gc_arena_save(mrb);
  mrb_str_buf_cat(mrb, dest, (const char*)src, (size_t)size);
  mrb_gc_arena_restore(mrb, ai);
  return size;
}

static mrb_value RGSSLoadData(mrb_state* mrb, const char* filename) {
  auto* io_service = lime::IOService::Instance();
  auto stream = io_service->OpenReadRaw(filename);
  auto raw_data = stream->ReadAll();

  return mrb_marshal_load(mrb, MarshalReaderString,
                          mrb_str_new(mrb, raw_data.data(), raw_data.size()));
}

MRB_FUNC(rgss_main) {
  mrb_value block;
  mrb_get_args(mrb, "&", &block);
  return mrb_funcall(mrb, block, "call", 0);
}

MRB_FUNC(rgss_stop) {
  for (;;)
    lime::Graphics::Instance()->Update();
  return mrb_nil_value();
}

MRB_FUNC(load_data) {
  const char* filename;
  mrb_get_args(mrb, "z", &filename);
  return RGSSLoadData(mrb, filename);
}

MRB_FUNC(save_data) {
  mrb_value data;
  const char* filename;
  mrb_get_args(mrb, "oz", &data, &filename);

  auto* io_service = lime::IOService::Instance();
  auto stream = io_service->OpenWrite(filename);

  auto ai = mrb_gc_arena_save(mrb);
  mrb_value str = mrb_str_new(mrb, NULL, 0);
  mrb_marshal_dump(mrb, data, MarshalWriterString, str, -1);
  stream->Write(RSTRING_PTR(str), RSTRING_LEN(str));
  mrb_gc_arena_restore(mrb, ai);

  return mrb_nil_value();
}

MRB_FUNC(kernel_exit) {
  mrb_raise(mrb, g_exit_exception, "exit");
  return mrb_nil_value();
}

// Split a decoded backtrace line "file:line[:in method]" into its filename
// and line-number parts.
static void ParseLocation(std::string_view text,
                          std::string& filename,
                          std::string& line_number) {
  auto first_colon = text.find(':');
  if (first_colon == std::string_view::npos)
    return;

  filename.assign(text.substr(0, first_colon));
  auto rest = text.substr(first_colon + 1);
  line_number.assign(rest.substr(0, rest.find(':')));
}

// Collect the pending mruby exception's class name, message, error location
// (filename/line) and the full call stack into the given output strings.
static void CollectExceptionInfo(mrb_state* mrb,
                                 std::string& err_class,
                                 std::string& err_message,
                                 std::string& err_filename,
                                 std::string& err_line,
                                 std::string& err_backtrace) {
  if (mrb->exc == nullptr)
    return;

  // Error class name
  const char* class_name = mrb_obj_classname(mrb, mrb_obj_value(mrb->exc));
  err_class = (class_name ? class_name : "(unknown)");

  // Error message
  auto* exc = (struct RException*)mrb->exc;
  mrb_value mesg = exc->mesg ? mrb_obj_value(exc->mesg) : mrb_nil_value();
  if (mrb_string_p(mesg))
    err_message.assign(RSTRING_PTR(mesg), (size_t)RSTRING_LEN(mesg));

  // Backtrace (most recent call last). The first frame is the location
  // where the exception was raised, so it also supplies filename/line.
  std::vector<std::string> lines;
  auto* backtrace = exc->backtrace;

  if (backtrace != nullptr && backtrace->tt == MRB_TT_BACKTRACE) {
    auto* bt = (struct RBacktrace*)backtrace;
    for (size_t i = 0; i < bt->len; ++i) {
      const auto& loc = bt->locations[i];

      int32_t line_no = -1;
      const char* file_name = nullptr;
      std::string text;
      if (loc.irep != nullptr &&
          mrb_debug_get_position(mrb, loc.irep, loc.idx, &line_no,
                                 &file_name) &&
          file_name != nullptr) {
        text = file_name;
        text += ':';
        text += std::to_string(line_no < 0 ? 0 : line_no);
      } else {
        text = "(unknown):0";
      }

      if (loc.method_id != 0) {
        text += ":in ";
        text += mrb_sym_name(mrb, loc.method_id);
      }

      if (i == 0)
        ParseLocation(text, err_filename, err_line);

      lines.push_back(std::move(text));
    }
  } else if (backtrace != nullptr && backtrace->tt == MRB_TT_ARRAY) {
    auto ary = mrb_obj_value(backtrace);
    for (mrb_int i = 0; i < RARRAY_LEN(ary); ++i) {
      mrb_value line = RARRAY_PTR(ary)[i];
      if (!mrb_string_p(line))
        continue;

      std::string text(RSTRING_PTR(line), (size_t)RSTRING_LEN(line));
      if (i == 0)
        ParseLocation(text, err_filename, err_line);

      lines.push_back(std::move(text));
    }
  }

  for (size_t i = 0; i < lines.size(); ++i) {
    if (i > 0)
      err_backtrace += '\n';
    err_backtrace += lines[i];
  }
}

void ShowExceptionMessage(std::string message) {
  auto screen_image = raylib::LoadImageFromScreen();
  raylib::ImageBlurGaussian(&screen_image, 5);
  auto screen_texture = raylib::LoadTextureFromImage(screen_image);

  for (;;) {
    raylib::BeginDrawing();
    {
      raylib::ClearBackground({});

      raylib::Rectangle src_rect = {}, dst_rect = {};
      src_rect.width = (float)screen_texture.width;
      src_rect.height = (float)screen_texture.height;
      dst_rect.width = (float)raylib::GetScreenWidth();
      dst_rect.height = (float)raylib::GetScreenHeight();
      raylib::DrawTexturePro(screen_texture, src_rect, dst_rect, {}, 0,
                             raylib::WHITE);

      raylib::DrawText(message.c_str(), 10, 10, 16, raylib::WHITE);

      auto dpi = raylib::GetWindowScaleDPI();
      raylib::Rectangle bound = {
          (float)raylib::GetScreenWidth() - 50 * dpi.x,
          (float)raylib::GetScreenHeight() - 30 * dpi.y,
          40 * dpi.x,
          20 * dpi.y,
      };

      if (raylib::GuiButton(bound, "OK"))
        break;
      if (raylib::WindowShouldClose())
        break;
    }
    raylib::EndDrawing();
  }

  raylib::UnloadTexture(screen_texture);
  raylib::UnloadImage(screen_image);
}

extern "C" void lime_main() {
  auto* config = lime::Config::Instance();
  auto* io_service = lime::IOService::Instance();

  // Global mruby state
  mrb_state* mrb = mrb_open();
  mrb_show_version(mrb);

  // Initialize bindings
  InitBindings(mrb);

  // Internal exception class
  g_reset_exception =
      mrb_define_class(mrb, "RGSSReset", mrb->eStandardError_class);
  g_rgss_exception =
      mrb_define_class(mrb, "RGSSError", mrb->eStandardError_class);
  g_exit_exception =
      mrb_define_class(mrb, "SystemExit", mrb->eStandardError_class);

  // Default functions
  mrb_define_module_function(mrb, mrb->kernel_module, "exit", kernel_exit,
                             MRB_ARGS_NONE());

  // Internal functions
  mrb_define_module_function(mrb, mrb->kernel_module, "rgss_main", rgss_main,
                             MRB_ARGS_BLOCK());
  mrb_define_module_function(mrb, mrb->kernel_module, "rgss_stop", rgss_stop,
                             MRB_ARGS_NONE());

  // Internal IO
  mrb_define_module_function(mrb, mrb->kernel_module, "load_data", load_data,
                             MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, mrb->kernel_module, "save_data", save_data,
                             MRB_ARGS_REQ(2));

  // Dir (CRuby-compatible, backed by the engine's virtual filesystem)
  InitStdlibDir(mrb);

  // RPG Database
  auto rpg_ctx = mrb_ccontext_new(mrb);
  mrb_ccontext_filename(mrb, rpg_ctx, "RPGDataStructure");
  if (config->rgss_version == 1)
    mrb_load_string_cxt(mrb, rpg_rgss1, rpg_ctx);
  else if (config->rgss_version == 2)
    mrb_load_string_cxt(mrb, rpg_rgss2, rpg_ctx);
  else if (config->rgss_version == 3)
    mrb_load_string_cxt(mrb, rpg_rgss3, rpg_ctx);
  mrb_ccontext_free(mrb, rpg_ctx);

  // Load and execute the main script
  try {
    // Load marshal data
    auto scripts = RGSSLoadData(mrb, config->scripts.c_str());
    if (mrb_type(scripts) != MRB_TT_ARRAY)
      throw lime::Exception(lime::Exception::RGSSError,
                            "scripts file is invalid.");

    // Decode scripts
    std::vector<uint8_t> scripts_buffer;
    scripts_buffer.resize(1024);
    for (int i = 0; i < RARRAY_LEN(scripts); ++i) {
      mrb_value script = mrb_ary_entry(scripts, i);
      if (mrb_type(script) != MRB_TT_ARRAY || RARRAY_LEN(script) < 3)
        continue;

      // 0 -> editor chunk
      // 1 -> script name
      // 2 -> script data (compressed)
      mrb_value script_string = mrb_ary_entry(script, 2);
      if (mrb_type(script_string) != MRB_TT_STRING)
        continue;

      const uint8_t* src_buffer = (const uint8_t*)RSTRING_PTR(script_string);
      uLongf src_size = (uLongf)RSTRING_LEN(script_string);

      // Decompress the script; when the output buffer is too small
      // (Z_BUF_ERROR) grow it and keep retrying until the decoded data fits.
      int result = 0;
      uLongf buffer_size = 0;
      for (;;) {
        buffer_size = (uLongf)scripts_buffer.size();
        result = ::uncompress(scripts_buffer.data(), &buffer_size, src_buffer,
                              src_size);

        if (result == Z_BUF_ERROR) {
          scripts_buffer.resize(scripts_buffer.size() + 1024);
        } else {
          break;
        }
      }

      // Any other result is a real decode failure (corrupt data, OOM, ...).
      if (result != Z_OK)
        throw lime::Exception(lime::Exception::RGSSError,
                              "failed to decompress script");

      // Trim to the exact decompressed size and evaluate the script.
      std::string_view script_view(
          reinterpret_cast<const char*>(scripts_buffer.data()), buffer_size);
      mrb_ary_set(mrb, script, 2,
                  mrb_str_new(mrb, script_view.data(), script_view.size()));
    }

    // Executing scripts
    auto cctx = mrb_ccontext_new(mrb);
    for (int i = 0; i < RARRAY_LEN(scripts); ++i) {
      auto script = mrb_ary_entry(scripts, i);
      auto script_name = mrb_ary_entry(script, 1);
      auto script_string = mrb_ary_entry(script, 2);

      auto ai = mrb_gc_arena_save(mrb);
      mrb_ccontext_filename(mrb, cctx, mrb_str_to_cstr(mrb, script_name));
      mrb_load_nstring_cxt(mrb, RSTRING_PTR(script_string),
                           RSTRING_LEN(script_string), cctx);
      mrb_gc_arena_restore(mrb, ai);

      if (mrb->exc)
        break;
    }
    mrb_ccontext_free(mrb, cctx);

    // Parsing exception if available
    if (mrb->exc && mrb->exc->c != g_exit_exception) {
      mrb_print_backtrace(mrb);

      // Collect the exception class name, message, error location
      // (filename/line) and the full call stack.
      std::string err_class, err_message, err_filename, err_line, err_backtrace;
      CollectExceptionInfo(mrb, err_class, err_message, err_filename, err_line,
                           err_backtrace);

      throw lime::Exception(lime::Exception::RGSSError,
                            "{}: {} ({} - line {})\n{}", err_class, err_message,
                            err_filename, err_line, err_backtrace);
    }
  } catch (const lime::Exception& e) {
    // Content exception
    ShowExceptionMessage(e.message());
  } catch (const std::exception& e) {
    // Internal exception
    ShowExceptionMessage(e.what());
  }

  // Finalize mruby
  mrb_close(mrb);
}

}  // namespace binding
