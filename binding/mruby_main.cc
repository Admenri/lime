
#include <algorithm>

#include "zlib.h"

#include "mruby_utils.h"

#include "3rdparty/mruby-cmake/gems/mruby-marshal-c/include/mruby/marshal.h"

#include "src/filesystem.h"
#include "src/graphics.h"
#include "src/profile.h"

#include "core/binding_init.h"

#include "rpg/rpg_rgss1.h"
#include "rpg/rpg_rgss2.h"
#include "rpg/rpg_rgss3.h"

namespace binding {

RClass* g_reset_exception = nullptr;
RClass* g_rgss_exception = nullptr;

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
  auto* io_service = rgssx::IOService::Instance();
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
    rgssx::Graphics::Instance()->Update();
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

  auto* io_service = rgssx::IOService::Instance();
  auto stream = io_service->OpenWrite(filename);

  auto ai = mrb_gc_arena_save(mrb);
  mrb_value str = mrb_str_new(mrb, NULL, 0);
  mrb_marshal_dump(mrb, data, MarshalWriterString, str, -1);
  stream->Write(RSTRING_PTR(str), RSTRING_LEN(str));
  mrb_gc_arena_restore(mrb, ai);

  return mrb_nil_value();
}

// Case-insensitive glob match: '*' matches any (possibly empty) sequence and
// '?' matches exactly one character. Used by Dir.glob for filename filtering.
static bool GlobMatch(const char* pattern, const char* name) {
  while (*pattern) {
    if (*pattern == '*') {
      // Collapse consecutive stars, then try to match the remainder at each
      // position of the name.
      while (*pattern == '*')
        ++pattern;
      if (*pattern == '\0')
        return true;
      while (*name) {
        if (GlobMatch(pattern, name))
          return true;
        ++name;
      }
      return false;
    }
    if (*name == '\0')
      return false;
    if (*pattern != '?' && std::tolower(static_cast<unsigned char>(*pattern)) !=
                               std::tolower(static_cast<unsigned char>(*name)))
      return false;
    ++pattern;
    ++name;
  }
  return *name == '\0';
}

MRB_FUNC(dir_glob) {
  const char* pattern;
  mrb_get_args(mrb, "z", &pattern);

  // Split the directory prefix from the filename pattern at the last '/', e.g.
  // "Save*.rvdata2" -> dir="", file="Save*.rvdata2".
  std::string pat(pattern);
  std::string dir, file_pat = pat;
  const size_t slash = pat.find_last_of('/');
  if (slash != std::string::npos) {
    dir = pat.substr(0, slash);
    file_pat = pat.substr(slash + 1);
  }

  // Enumerate the directory and keep the entries matching the pattern.
  std::vector<std::string> matches;
  for (const auto& name : rgssx::IOService::Instance()->EnumDir(dir)) {
    if (GlobMatch(file_pat.c_str(), name.c_str()))
      matches.push_back(dir.empty() ? name : dir + "/" + name);
  }
  std::sort(matches.begin(), matches.end());

  auto result = mrb_ary_new(mrb);
  for (const auto& match : matches)
    mrb_ary_push(mrb, result, mrb_str_new_cstr(mrb, match.c_str()));
  return result;
}

extern "C" void rgssx_main() {
  auto* config = rgssx::Config::Instance();
  auto* io_service = rgssx::IOService::Instance();

  // Global mruby state
  mrb_state* mrb = mrb_open();

  // Initialize bindings
  mrb_show_version(mrb);
  InitBindings(mrb);

  // Internal exception class
  g_reset_exception = mrb_define_class(mrb, "RGSSReset", mrb->eException_class);
  g_rgss_exception = mrb_define_class(mrb, "RGSSError", mrb->eException_class);

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

  // Dir
  auto module_dir = mrb_define_module(mrb, "Dir");
  mrb_define_module_function(mrb, module_dir, "glob", dir_glob,
                             MRB_ARGS_REQ(1));

  // RPG Database
  if (config->rgss_version == 1)
    mrb_load_string(mrb, rpg_rgss1);
  else if (config->rgss_version == 2)
    mrb_load_string(mrb, rpg_rgss2);
  else if (config->rgss_version == 3)
    mrb_load_string(mrb, rpg_rgss3);

  // Load and execute the main script
  try {
    auto scripts = RGSSLoadData(mrb, config->scripts.c_str());

    if (mrb_type(scripts) != MRB_TT_ARRAY)
      throw rgssx::Exception("scripts file is invalid.");

    std::vector<uint8_t> scripts_buffer(1 << 16, 0);
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
          scripts_buffer.resize(scripts_buffer.size() + 1 << 16);
        } else {
          break;
        }
      }

      // Any other result is a real decode failure (corrupt data, OOM, ...).
      if (result != Z_OK)
        throw rgssx::Exception("failed to decompress script");

      // Trim to the exact decompressed size and evaluate the script.
      std::string_view script_view(
          reinterpret_cast<const char*>(scripts_buffer.data()), buffer_size);
      mrb_ary_set(mrb, script, 2,
                  mrb_str_new(mrb, script_view.data(), script_view.size()));
    }

    for (;;) {
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

      if (mrb->exc) {
        mrb_print_backtrace(mrb);

        auto exc_value = mrb_obj_value(mrb->exc);
        auto* exc_klass = (RException*)mrb->exc;

        std::string exc_msg;
        if (exc_klass->mesg)
          exc_msg = mrb_str_to_cstr(mrb, mrb_obj_value(exc_klass->mesg));
        std::string exc_class = mrb_class_name(mrb, mrb_class(mrb, exc_value));

        throw rgssx::Exception("{}: {}", exc_class, exc_msg);
      }

      break;
    }
  } catch (const std::exception& e) {
    raylib::TraceLog(raylib::LOG_ERROR, "Exception: %s", e.what());
  }

  // Finalize mruby
  mrb_close(mrb);
}

}  // namespace binding
