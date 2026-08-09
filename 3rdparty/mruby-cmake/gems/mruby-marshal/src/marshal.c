#include <mruby.h>
#include <mruby/value.h>
#include <mruby/marshal.h>
#include <mruby/class.h>
#include <mruby/string.h>
#include <mruby/presym.h>

#include "common.h"
#include <string.h>

#if defined(_WIN32)
#include <io.h>
#include <fcntl.h>
#endif

static int
_writer_string(mrb_state *mrb, const void *src, int size, mrb_value dest, mrb_uint position)
{
  int ai = mrb_gc_arena_save(mrb);
  mrb_str_buf_cat(mrb, dest, (const char *)src, (size_t)size);
  mrb_gc_arena_restore(mrb, ai);
  return size;
}

static int
_writer_io(mrb_state *mrb, const void *src, int size, mrb_value dest, mrb_uint position)
{
  int ai = mrb_gc_arena_save(mrb);
  int written = mrb_as_int(mrb, mrb_funcall_id(mrb, dest, s_write, 1, mrb_str_new(mrb, (const char *)src, size)));
  mrb_gc_arena_restore(mrb, ai);
  return written;
}

#if defined(_WIN32)
/* Marshal data is binary. mruby-io's File.open(path, "r") (no "b") opens in
 * Windows text mode, where _read() treats 0x1A (Ctrl-Z) as an end-of-file
 * marker and _read()/_write() translate CRLF -- both corrupt binary save
 * data. CRuby reads files in binary mode by default, so force the underlying
 * fd to binary here to match that behavior. */
static void
ensure_binary_io(mrb_state *mrb, mrb_value io)
{
  if (mrb_respond_to(mrb, io, mrb_intern_lit(mrb, "fileno")))
  {
    mrb_value fd_v = mrb_funcall_id(mrb, io, mrb_intern_lit(mrb, "fileno"), 0);
    if (mrb_fixnum_p(fd_v))
    {
      _setmode((int)mrb_fixnum(fd_v), _O_BINARY);
    }
  }
}
#endif

static mrb_value
mrb_mruby_marshal_dump(mrb_state *mrb, mrb_value self)
{
  mrb_value obj, io = mrb_nil_value();
  mrb_int limit = -1;
  const mrb_int arg_count = mrb_get_args(mrb, "o|oi", &obj, &io, &limit);
  if (arg_count == 2 && mrb_fixnum_p(io))
  {
    limit = mrb_fixnum(io);
    io = mrb_nil_value();
  }
  if (mrb_nil_p(io))
  {
    mrb_value str = mrb_str_new(mrb, NULL, 0);
    mrb_marshal_dump(mrb, obj, _writer_string, str, limit);
    return str;
  }
  else
  {
#if defined(_WIN32)
    ensure_binary_io(mrb, io);
#endif
    mrb_marshal_dump(mrb, obj, _writer_io, io, limit);
    return io;
  }
}

static int
_reader_string(mrb_state *mrb, mrb_value src, void *dest, int size, mrb_uint position)
{
  int remain = RSTRING_LEN(src) - position;
  if (size < 0)
  {
    mrb_raisef(mrb, E_ARGUMENT_ERROR, "negative length %d given", size);
  }
  if (remain > 0)
  {
    int len = remain < size ? remain : size;
    memcpy(dest, RSTRING_PTR(src) + position, len);
    return len;
  }
  return 0;
}

static int
_reader_io(mrb_state *mrb, mrb_value src, void *dest, int size, mrb_uint position)
{
  int ai = mrb_gc_arena_save(mrb);
  /* Read sequentially from the IO's current position. We must NOT seek to an
   * absolute `position`: load.c resets the position counter to 0 for every
   * Marshal.load() call, so seeking would make every call re-read the first
   * object from the start of the file. That breaks multi-object streams (e.g.
   * a save file loaded with several consecutive Marshal.load(file) calls),
   * which is what CRuby supports: each call continues from where the previous
   * one left off. */
  (void)position;
  mrb_value buf = mrb_funcall_id(mrb, src, s_read, 1, mrb_fixnum_value(size));
  int buf_len = 0;
  if (mrb_string_p(buf))
  {
    memcpy(dest, RSTRING_PTR(buf), RSTRING_LEN(buf));
    buf_len = RSTRING_LEN(buf);
  }
  mrb_gc_arena_restore(mrb, ai);
  return buf_len;
}

static mrb_value
mrb_mruby_marshal_load(mrb_state *mrb, mrb_value self)
{
  mrb_value obj;
  mrb_get_args(mrb, "o", &obj);
  if (mrb_string_p(obj))
  {
    return mrb_marshal_load(mrb, _reader_string, obj);
  }
#if defined(_WIN32)
  ensure_binary_io(mrb, obj);
#endif
  return mrb_marshal_load(mrb, _reader_io, obj);
}

void mrb_mruby_marshal_gem_init(mrb_state *mrb)
{
  struct RClass *mrb_marshal;
  mrb_marshal = mrb_define_module_id(mrb, MRB_SYM(Marshal));

  mrb_define_module_function_id(mrb, mrb_marshal, MRB_SYM(dump), mrb_mruby_marshal_dump, MRB_ARGS_REQ(1) | MRB_ARGS_OPT(2));
  mrb_define_module_function_id(mrb, mrb_marshal, MRB_SYM(load), mrb_mruby_marshal_load, MRB_ARGS_REQ(1));
  mrb_define_module_function_id(mrb, mrb_marshal, MRB_SYM(restore), mrb_mruby_marshal_load, MRB_ARGS_REQ(1));

  mrb_define_const_id(mrb, mrb_marshal, MRB_SYM(MAJOR_VERSION), mrb_fixnum_value(MARSHAL_MAJOR));
  mrb_define_const_id(mrb, mrb_marshal, MRB_SYM(MINOR_VERSION), mrb_fixnum_value(MARSHAL_MINOR));
}

void mrb_mruby_marshal_gem_final(mrb_state *mrb)
{
}
