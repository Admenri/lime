
#include "zlib.h"

#include "mruby.h"
#include "mruby/array.h"
#include "mruby/class.h"
#include "mruby/compile.h"
#include "mruby/data.h"
#include "mruby/error.h"
#include "mruby/string.h"
#include "mruby/variable.h"
#include "mruby/version.h"

#include "3rdparty/mruby-cmake/gems/mruby-marshal-c/include/mruby/marshal.h"

#include "config.h"
#include "filesystem.h"

#include "core/binding_init.h"

namespace binding {}

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
    memcpy(dest, RSTRING_PTR(src) + position, len);
    return len;
  }

  return 0;
}

extern "C" void rgssx_main() {
  auto* config = rgssx::Config::Instance();
  auto* io_service = rgssx::IOService::Instance();

  // Initialize mruby and bindings
  mrb_state* mrb = mrb_open();
  auto* cctx = mrb_ccontext_new(mrb);

  mrb_show_version(mrb);
  binding::InitBindings(mrb);

  // Load and execute the main script
  try {
    auto stream = io_service->OpenReadRaw(config->scripts);
    auto raw_scripts_marshal = rgssx::ReadStream(*stream);

    auto scripts = mrb_marshal_load(mrb, MarshalReaderString,
                                    mrb_str_new(mrb, raw_scripts_marshal.data(),
                                                raw_scripts_marshal.size()));

    if (mrb_type(scripts) != MRB_TT_ARRAY)
      throw std::runtime_error("scripts file is invalid.");

    std::vector<uint8_t> scripts_buffer(1 << 16, 0);
    for (int i = 0; i < RARRAY_LEN(scripts); ++i) {
      mrb_value script = mrb_ary_entry(scripts, i);
      if (mrb_type(script) != MRB_TT_ARRAY || RARRAY_LEN(script) < 3)
        continue;

      mrb_value script_name = mrb_ary_entry(script, 1);
      mrb_value script_string = mrb_ary_entry(script, 2);
      if (mrb_type(script_string) != MRB_TT_STRING)
        continue;

      raylib::TraceLog(raylib::LOG_INFO, "Executing script: %s",
                       mrb_str_to_cstr(mrb, script_name));

      const uint8_t* src_buffer = (const uint8_t*)RSTRING_PTR(script_string);
      uLongf src_size = (uLongf)RSTRING_LEN(script_string);

      // Decompress the script; when the output buffer is too small
      // (Z_BUF_ERROR) grow it and keep retrying until the decoded data fits.
      int result = 0;
      uLongf buffer_size = 0;
      for (;;) {
        buffer_size = (uLongf)scripts_buffer.size();
        result = uncompress(scripts_buffer.data(), &buffer_size, src_buffer,
                            src_size);

        if (result == Z_BUF_ERROR) {
          scripts_buffer.resize(scripts_buffer.size() + 1 << 16);
        } else {
          break;
        }
      }

      // Any other result is a real decode failure (corrupt data, OOM, ...).
      if (result != Z_OK)
        throw std::runtime_error("failed to decompress script");

      // Trim to the exact decompressed size and evaluate the script.
      mrb_ccontext_filename(mrb, cctx, mrb_str_to_cstr(mrb, script_name));
      std::string_view scripts_buffer_view(
          reinterpret_cast<const char*>(scripts_buffer.data()), buffer_size);
      mrb_load_nstring_cxt(mrb, scripts_buffer_view.data(),
                           scripts_buffer_view.size(), cctx);
      if (mrb->exc) {
        mrb_print_error(mrb);
        mrb->exc = NULL;
      }
    }
  } catch (const std::exception& e) {
    raylib::TraceLog(raylib::LOG_ERROR, "Exception: %s", e.what());
  }

  // Finalize mruby
  mrb_ccontext_free(mrb, cctx);
  mrb_close(mrb);
}
