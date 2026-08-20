// Marshal.dump / Marshal.load for mruby — port of CRuby's marshal.c (Ruby 4.0.6).
//
// Compatibility notes:
//  - Strings/symbols dumped plain (no encoding wrappers); MRI encoding ivars
//    consumed as no-ops on load.
//  - Ranges: TYPE_OBJECT with "excl","begin","end" ivars; rebuilt via mrb_range_new.
//  - Floats: shortest round-trip via std::to_chars + legacy mantissa decoder.
//  - Bignums: |fixnum| >= 2^31 dumps as TYPE_BIGNUM; >int64 raises RangeError.
//  - Hash ivars: symbol-hash order (not insertion); round-trip guaranteed.
//  - Regexp: TYPE_REGEXP rebuilt via Regexp.new(source, options).
//  - Missing classes raise NameError; TYPE_UCLASS reclassifies via class ptr.

#include "mruby/marshal.h"

#include <mruby/array.h>
#include <mruby/class.h>
#include <mruby/error.h>
#include <mruby/hash.h>
#include <mruby/numeric.h>
#include <mruby/range.h>
#include <mruby/string.h>
#include <mruby/variable.h>

#include <charconv>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#define MARSHAL_MAJOR 4
#define MARSHAL_MINOR 8

#define TYPE_NIL '0'
#define TYPE_TRUE 'T'
#define TYPE_FALSE 'F'
#define TYPE_FIXNUM 'i'
#define TYPE_EXTENDED 'e'
#define TYPE_UCLASS 'C'
#define TYPE_OBJECT 'o'
#define TYPE_DATA 'd'
#define TYPE_USERDEF 'u'
#define TYPE_USRMARSHAL 'U'
#define TYPE_FLOAT 'f'
#define TYPE_BIGNUM 'l'
#define TYPE_STRING '"'
#define TYPE_REGEXP '/'
#define TYPE_ARRAY '['
#define TYPE_HASH '{'
#define TYPE_HASH_DEF '}'
#define TYPE_STRUCT 'S'
#define TYPE_MODULE_OLD 'M'
#define TYPE_CLASS 'c'
#define TYPE_MODULE 'm'
#define TYPE_SYMBOL ':'
#define TYPE_SYMLINK ';'
#define TYPE_IVAR 'I'
#define TYPE_LINK '@'

#define BUFSIZ_FLUSH 8192
#define DECIMAL_MANT (53 - 16)  // from IEEE754 double precision
#define MANT_BITS 32

static mrb_sym s_dump, s_load, s_mdump, s_mload;
static mrb_sym s_dump_data, s_load_data;
static mrb_sym s_call, s_getbyte, s_read, s_write, s_binmode;
static mrb_sym s_allocate, s_extend, s_prepend, s_to_str;
static mrb_sym s_excl, s_begin, s_end, s_new;
static mrb_sym s_source, s_options;

namespace {

// ---------------------------------------------------------------------------
// GC: mruby does not scan the C stack. Every object that must outlive an
// allocation or funcall inside this file is registered with mrb_gc_register
// and unregistered by clear_*(). Register/unregister counts are balanced.
// The final result of each protected body is additionally kept alive by
// mrb_gc_protect() inside mrb_protect_error (vm.c), so clearing before
// returning it to the VM is safe.
// ---------------------------------------------------------------------------

static void gc_roots_push(mrb_state *mrb, std::vector<mrb_value> *roots, mrb_value v)
{
  if (mrb_immediate_p(v)) return;
  roots->push_back(v);
  mrb_gc_register(mrb, v);
}

static void gc_roots_clear(mrb_state *mrb, std::vector<mrb_value> *roots)
{
  for (mrb_value v : *roots) {
    mrb_gc_unregister(mrb, v);
  }
  roots->clear();
}

struct dump_arg {
  mrb_state *mrb = nullptr;
  mrb_value str;                                  // output buffer
  mrb_value dest;                                 // IO target or nil
  std::unordered_map<uintptr_t, mrb_int> symbols; // mrb_sym -> index
  std::unordered_map<uintptr_t, mrb_int> data;    // object ptr -> index
  std::unordered_set<uintptr_t> userdefs;
  std::vector<mrb_value> gc_roots;
  mrb_int num_entries = 0;
  bool active = false;
};

struct dump_call_arg {
  dump_arg *arg;
  mrb_int limit;
};

struct load_arg {
  mrb_state *mrb = nullptr;
  mrb_value src;                                  // String or IO
  bool is_io = false;                             // src is an IO (not String)
  mrb_int offset = 0;                             // String source cursor
  std::vector<mrb_value> data;                    // index -> object (undef = link pending)
  std::unordered_map<mrb_int, mrb_value> symbols; // index -> name string
  std::unordered_set<struct RBasic *> partial_objects;
  std::vector<mrb_value> gc_roots;
  mrb_value proc;
  bool active = false;
};

static void clear_dump_arg(dump_arg *arg)
{
  if (!arg->active) return;
  arg->active = false;
  gc_roots_clear(arg->mrb, &arg->gc_roots);
  arg->symbols.clear();
  arg->data.clear();
  arg->userdefs.clear();
}

static void clear_load_arg(load_arg *arg)
{
  if (!arg->active) return;
  arg->active = false;
  gc_roots_clear(arg->mrb, &arg->gc_roots);
  arg->data.clear();
  arg->symbols.clear();
  arg->partial_objects.clear();
}

// ---------------------------------------------------------------------------
// Write helpers
// ---------------------------------------------------------------------------

static void w_nbyte(dump_arg *arg, const char *s, size_t n)
{
  mrb_state *mrb = arg->mrb;
  mrb_str_cat(mrb, arg->str, s, (mrb_int)n);
  if (!mrb_nil_p(arg->dest) && RSTRING_LEN(arg->str) >= BUFSIZ_FLUSH) {
    mrb_value args[1] = { arg->str };
    mrb_funcall_argv(mrb, arg->dest, s_write, 1, args);
    mrb_str_resize(mrb, arg->str, 0);
  }
}

static void w_byte(dump_arg *arg, char c)
{
  w_nbyte(arg, &c, 1);
}

static void w_long(mrb_int x, dump_arg *arg);

static void w_bytes(dump_arg *arg, const char *s, size_t n)
{
  w_long((mrb_int)n, arg);
  w_nbyte(arg, s, n);
}

static void w_short(dump_arg *arg, mrb_int x)
{
  w_byte(arg, (char)((x >> 0) & 0xff));
  w_byte(arg, (char)((x >> 8) & 0xff));
}

// Mirrors ruby_marshal_write_long (4.0.6): only 31-bit values fit.
static void w_long(mrb_int x, dump_arg *arg)
{
  mrb_state *mrb = arg->mrb;
  char buf[sizeof(mrb_int) + 1];
  mrb_int i;

  if (!(x >> 31 == 0 || x >> 31 == -1)) {
    mrb_raise(mrb, E_TYPE_ERROR, "long too big to dump");
  }
  if (x == 0) {
    buf[0] = 0;
    i = 1;
  }
  else if (0 < x && x < 123) {
    buf[0] = (char)(x + 5);
    i = 1;
  }
  else if (-124 < x && x < 0) {
    buf[0] = (char)((x - 5) & 0xff);
    i = 1;
  }
  else {
    for (i = 1; i < (mrb_int)sizeof(mrb_int) + 1; i++) {
      buf[i] = (char)(x & 0xff);
      x >>= 8;
      if (x == 0) {
        buf[0] = (char)i;
        break;
      }
      if (x == -1) {
        buf[0] = (char)-i;
        break;
      }
    }
    i++;
  }
  w_nbyte(arg, buf, (size_t)i);
}

// ---------------------------------------------------------------------------
// Floats: shortest round-trip decimal (CRuby dtoa mode 0) via std::to_chars,
// formatted with CRuby's w_float layout rules.
// ---------------------------------------------------------------------------

static std::string shortest_float_string(double f)
{
  if (f == 0.0) {
    return std::signbit(f) ? "-0" : "0";
  }

  char buf[128];
  auto res = std::to_chars(buf, buf + sizeof(buf), f);
  if (res.ec != std::errc()) {
    return "0";  // unreachable
  }
  std::string s(buf, res.ptr);
  bool neg = (s[0] == '-');
  if (neg) s = s.substr(1);

  size_t ep = s.find_first_of("eE");
  std::string mant = s;
  int expn = 0;
  if (ep != std::string::npos) {
    mant = s.substr(0, ep);
    expn = atoi(s.c_str() + ep + 1);
  }

  size_t ip = mant.find('.');
  std::string intp = ip == std::string::npos ? mant : mant.substr(0, ip);
  std::string frac = ip == std::string::npos ? "" : mant.substr(ip + 1);
  int decpt = expn + (int)intp.size();
  std::string digits = intp + frac;

  // strip leading zeros (fractional-only numbers, e.g. 0.00123)
  size_t z = 0;
  while (z < digits.size() && digits[z] == '0') z++;
  if (z > 0) {
    if (z == digits.size()) return "0";
    decpt -= (int)z;
    digits = digits.substr(z);
  }
  // strip trailing zeros (dtoa mode 0 emits none; to_chars emits e.g. "100")
  while (digits.size() > 1 && digits.back() == '0') digits.pop_back();

  std::string out;
  if (decpt < -3 || decpt > (int)digits.size()) {
    out += digits[0];
    if (digits.size() > 1) {
      out += '.';
      out += digits.substr(1);
    }
    char eb[24];
    snprintf(eb, sizeof(eb), "e%d", decpt - 1);
    out += eb;
  }
  else if (decpt > 0) {
    out = digits.substr(0, (size_t)decpt);
    if (decpt < (int)digits.size()) {
      out += '.';
      out += digits.substr((size_t)decpt);
    }
  }
  else {
    out = "0.";
    out.append((size_t)-decpt, '0');
    out += digits;
  }
  return neg ? "-" + out : out;
}

static void w_float(mrb_state *mrb, mrb_float d, dump_arg *arg)
{
  if (isinf(d)) {
    if (d < 0) w_bytes(arg, "-inf", 4);
    else w_bytes(arg, "inf", 3);
  }
  else if (isnan(d)) {
    w_bytes(arg, "nan", 3);
  }
  else {
    std::string s = shortest_float_string(d);
    w_bytes(arg, s.data(), s.size());
  }
}

// Legacy binary mantissa format (pre-1.8 floats), ported verbatim from
// CRuby 4.0.6 load_mantissa (no fall-through).
static double load_mantissa(double d, const char *buf, long len)
{
  if (!len) return d;
  if (--len > 0 && !*buf++) {  // binary mantissa mark
    int e, s = d < 0, dig = 0;
    unsigned long m;

    modf(ldexp(frexp(fabs(d), &e), DECIMAL_MANT), &d);
    do {
      m = 0;
      if (len >= 4) {
        m = (unsigned char)*buf++;
        m = (m << 8) | (unsigned char)*buf++;
        m = (m << 8) | (unsigned char)*buf++;
        m = (m << 8) | (unsigned char)*buf++;
      }
      else if (len == 3) {
        m = (unsigned char)*buf++;
        m = (m << 8) | (unsigned char)*buf++;
        m = (m << 8) | (unsigned char)*buf++;
      }
      else if (len == 2) {
        m = (unsigned char)*buf++;
        m = (m << 8) | (unsigned char)*buf++;
      }
      else {
        m = (unsigned char)*buf++;
      }
      dig -= len < MANT_BITS / 8 ? 8 * (unsigned)len : MANT_BITS;
      d += ldexp((double)m, dig);
    } while ((len -= MANT_BITS / 8) > 0);
    d = ldexp(d, e - DECIMAL_MANT);
    if (s) d = -d;
  }
  return d;
}

// ---------------------------------------------------------------------------
// Misc dump-side helpers
// ---------------------------------------------------------------------------

static mrb_value check_dump_arg(mrb_state *mrb, mrb_value ret, dump_arg *arg, const char *name)
{
  if (!arg->active) {
    mrb_raisef(mrb, E_RUNTIME_ERROR, "Marshal.dump reentered at %s", name);
  }
  return ret;
}

#define dump_funcall(mrb, arg, obj, sym, argc, argv) \
  check_dump_arg(mrb, mrb_funcall_argv(mrb, obj, sym, argc, argv), arg, mrb_sym_name(mrb, sym))

static mrb_value class2path(mrb_state *mrb, struct RClass *klass)
{
  mrb_value path = mrb_class_path(mrb, klass);
  if (mrb_nil_p(path)) {
    const char *type = mrb_type(mrb_obj_value(klass)) == MRB_TT_MODULE ? "module" : "class";
    mrb_raisef(mrb, E_TYPE_ERROR, "can't dump anonymous %s", type);
  }
  return path;
}

static void must_not_be_anonymous(mrb_state *mrb, const char *type, mrb_value path)
{
  const char *n = RSTRING_PTR(path);
  if (RSTRING_LEN(path) > 0 && n[0] == '#') {
    mrb_raisef(mrb, E_TYPE_ERROR, "can't dump anonymous %s %S", type, path);
  }
}

static void w_symbol(mrb_state *mrb, mrb_sym sym, dump_arg *arg)
{
  auto it = arg->symbols.find((uintptr_t)sym);
  if (it != arg->symbols.end()) {
    w_byte(arg, TYPE_SYMLINK);
    w_long(it->second, arg);
    return;
  }
  mrb_int len;
  const char *name = mrb_sym_name_len(mrb, sym, &len);
  w_byte(arg, TYPE_SYMBOL);
  w_bytes(arg, name, (size_t)len);
  arg->symbols[(uintptr_t)sym] = (mrb_int)arg->symbols.size();
}

static void w_unique(mrb_state *mrb, mrb_value s, dump_arg *arg)
{
  must_not_be_anonymous(mrb, "class", s);
  w_symbol(mrb, mrb_intern_str(mrb, s), arg);
}

static void w_object(mrb_state *mrb, mrb_value obj, dump_arg *arg, mrb_int limit);

static int hash_each(mrb_state *mrb, mrb_value key, mrb_value value, void *v)
{
  dump_call_arg *arg = (dump_call_arg *)v;
  w_object(mrb, key, arg->arg, arg->limit);
  w_object(mrb, value, arg->arg, arg->limit);
  return 0;
}

// Only real Ruby ivars (starting with '@') are serialized.
// Skip: mruby-internal ivars, Hash 'ifnone' (carried by TYPE_HASH_DEF),
// MRI encoding markers ('E', 'K', 'encoding'), singleton markers.
static bool is_skipped_ivar_name(const char *name, mrb_int len)
{
  return len < 1 || name[0] != '@';
}

static int obj_count_ivars(mrb_state *mrb, mrb_sym sym, mrb_value value, void *p)
{
  mrb_int len;
  const char *name = mrb_sym_name_len(mrb, sym, &len);
  if (!is_skipped_ivar_name(name, len)) {
    mrb_int *n = (mrb_int *)p;
    (*n)++;
  }
  return 0;
}

static mrb_int count_ivars(mrb_state *mrb, mrb_value obj)
{
  mrb_int n = 0;
  mrb_iv_foreach(mrb, obj, obj_count_ivars, &n);
  return n;
}

struct w_ivar_arg {
  dump_call_arg *dump;
};

static int w_obj_each(mrb_state *mrb, mrb_sym id, mrb_value value, void *a)
{
  struct w_ivar_arg *ivarg = (struct w_ivar_arg *)a;
  mrb_int len;
  const char *name = mrb_sym_name_len(mrb, id, &len);

  if (is_skipped_ivar_name(name, len)) {
    return 0;
  }
  w_symbol(mrb, id, ivarg->dump->arg);
  w_object(mrb, value, ivarg->dump->arg, ivarg->dump->limit);
  return 0;
}

static void w_ivar_each(mrb_state *mrb, mrb_value obj, dump_call_arg *arg)
{
  struct w_ivar_arg ivarg = { arg };
  mrb_iv_foreach(mrb, obj, w_obj_each, &ivarg);
}

static void w_ivar(mrb_state *mrb, mrb_int num, mrb_value ivobj, dump_call_arg *arg)
{
  w_long(num, arg->arg);
  w_ivar_each(mrb, ivobj, arg);
}

static void w_objivar(mrb_state *mrb, mrb_value obj, dump_call_arg *arg)
{
  mrb_int num = count_ivars(mrb, obj);
  w_long(num, arg->arg);
  w_ivar_each(mrb, obj, arg);
}

// True if the object would need the 'I' wrapper (ivars counted here, outside
// the type-specific dump).
static mrb_int has_ivars(mrb_state *mrb, mrb_value obj, mrb_value *ivobj)
{
  mrb_int num = 0;
  if (mrb_immediate_p(obj)) return 0;
  switch (mrb_type(obj)) {
    case MRB_TT_OBJECT:
    case MRB_TT_CLASS:
    case MRB_TT_MODULE:
    case MRB_TT_EXCEPTION:
    case MRB_TT_RANGE:
      break;  // counted elsewhere
    default:
      num = count_ivars(mrb, obj);
      if (num > 0) *ivobj = obj;
  }
  return num;
}

static void w_bigfixnum(mrb_state *mrb, mrb_value obj, dump_arg *arg)
{
  mrb_int num = mrb_integer(obj);
  char sign = num < 0 ? '-' : '+';
  mrb_uint mag = num < 0 ? (mrb_uint)(-(num + 1)) + 1 : (mrb_uint)num;

  w_byte(arg, TYPE_BIGNUM);
  w_byte(arg, sign);

  mrb_int slen = 0;
  for (mrb_uint n = mag; n; n >>= 16) slen++;
  w_long(slen, arg);
  for (mrb_int i = 0; i < slen; i++) {
    w_short(arg, (mrb_int)(mag & 0xffff));
    mag >>= 16;
  }

  // Not added to the link table, but the index still advances (4.0.6).
  arg->num_entries++;
}

static void w_remember(mrb_state *mrb, mrb_value obj, dump_arg *arg)
{
  arg->data[(uintptr_t)mrb_ptr(obj)] = arg->num_entries++;
  gc_roots_push(mrb, &arg->gc_roots, obj);
}

static bool singleton_dump_unable(mrb_state *mrb, struct RClass *klass)
{
  if (klass->mt && klass->mt->size > 0) return true;
  if (mrb_obj_class(mrb, mrb_obj_value(klass)) == NULL) return true;
  return count_ivars(mrb, mrb_obj_value(klass)) > 0;
}

static void w_extended(mrb_state *mrb, struct RClass *klass, dump_arg *arg, int check)
{
  if (check && mrb_type(mrb_obj_value(klass)) == MRB_TT_SCLASS) {
    if (singleton_dump_unable(mrb, klass)) {
      mrb_raise(mrb, E_TYPE_ERROR, "singleton can't be dumped");
    }
    klass = klass->super;
  }
  while (klass && mrb_type(mrb_obj_value(klass)) == MRB_TT_ICLASS) {
    if (!(klass->flags & MRB_FL_CLASS_IS_ORIGIN) ||
        mrb_type(mrb_obj_value(klass->c)) != MRB_TT_MODULE) {
      mrb_value path = class2path(mrb, (struct RClass *)klass->c);
      w_byte(arg, TYPE_EXTENDED);
      w_unique(mrb, path, arg);
    }
    klass = klass->super;
  }
}

static bool str_valid_utf8(const char *s, mrb_int len)
{
  for (mrb_int i = 0; i < len; i++) {
    unsigned char c = (unsigned char)s[i];
    if (c < 0x80) continue;
    mrb_int n;
    if ((c & 0xE0) == 0xC0) n = 1;
    else if ((c & 0xF0) == 0xE0) n = 2;
    else if ((c & 0xF8) == 0xF0) n = 3;
    else return false;
    if (i + n >= len) return false;
    for (mrb_int j = 1; j <= n; j++) {
      if (((unsigned char)s[i + j] & 0xC0) != 0x80) return false;
    }
    i += n;
  }
  return true;
}

static void w_class(mrb_state *mrb, char type, mrb_value obj, dump_arg *arg, int check)
{
  struct RClass *klass = mrb_class(mrb, obj);

  w_extended(mrb, klass, arg, check);
  w_byte(arg, type);
  mrb_value path = class2path(mrb, mrb_class_real(klass));
  w_unique(mrb, path, arg);
}

static void w_uclass(mrb_state *mrb, mrb_value obj, struct RClass *super, dump_arg *arg)
{
  struct RClass *klass = mrb_class(mrb, obj);

  w_extended(mrb, klass, arg, TRUE);
  klass = mrb_class_real(klass);
  if (klass != super) {
    w_byte(arg, TYPE_UCLASS);
    w_unique(mrb, class2path(mrb, klass), arg);
  }
}

static void w_object(mrb_state *mrb, mrb_value obj, dump_arg *arg, mrb_int limit)
{
  dump_call_arg c_arg;
  mrb_value ivobj = mrb_undef_value();
  mrb_int hasiv = 0;

  if (limit == 0) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "exceed depth limit");
  }

  if (mrb_nil_p(obj)) {
    w_byte(arg, TYPE_NIL);
    return;
  }
  if (mrb_true_p(obj)) {
    w_byte(arg, TYPE_TRUE);
    return;
  }
  if (mrb_false_p(obj)) {
    w_byte(arg, TYPE_FALSE);
    return;
  }
  if (mrb_integer_p(obj)) {
    mrb_int x = mrb_integer(obj);
    if (x >> 30 == 0 || x >> 30 == -1) {
      w_byte(arg, TYPE_FIXNUM);
      w_long(x, arg);
    }
    else {
      w_bigfixnum(mrb, obj, arg);
    }
    return;
  }
  if (mrb_symbol_p(obj)) {
    w_symbol(mrb, mrb_symbol(obj), arg);
    return;
  }

  auto it = arg->data.find((uintptr_t)mrb_ptr(obj));
  if (it != arg->data.end()) {
    w_byte(arg, TYPE_LINK);
    w_long(it->second, arg);
    return;
  }

  if (limit > 0) limit--;
  c_arg.arg = arg;
  c_arg.limit = limit;

  if (mrb_float_p(obj)) {
    w_remember(mrb, obj, arg);
    w_byte(arg, TYPE_FLOAT);
    w_float(mrb, mrb_float(obj), arg);
    return;
  }

  mrb_value v;

  if (mrb_respond_to(mrb, obj, s_mdump)) {
    w_remember(mrb, obj, arg);
    v = dump_funcall(mrb, arg, obj, s_mdump, 0, 0);
    gc_roots_push(mrb, &arg->gc_roots, v);
    w_class(mrb, TYPE_USRMARSHAL, obj, arg, FALSE);
    w_object(mrb, v, arg, limit);
    return;
  }
  if (mrb_respond_to(mrb, obj, s_dump)) {
    if (arg->userdefs.count((uintptr_t)mrb_ptr(obj)) > 0) {
      mrb_raise(mrb, E_RUNTIME_ERROR, "can't dump recursive object using _dump()");
    }
    mrb_value lv = mrb_fixnum_value(limit);
    v = dump_funcall(mrb, arg, obj, s_dump, 1, &lv);
    if (!mrb_string_p(v)) {
      mrb_raise(mrb, E_TYPE_ERROR, "_dump() must return string");
    }
    gc_roots_push(mrb, &arg->gc_roots, v);
    hasiv = has_ivars(mrb, obj, &ivobj);
    mrb_value ivobj2 = mrb_undef_value();
    mrb_int hasiv2 = has_ivars(mrb, v, &ivobj2);
    if (hasiv2) {
      hasiv = hasiv2;
      ivobj = ivobj2;
    }
    if (hasiv) w_byte(arg, TYPE_IVAR);
    w_class(mrb, TYPE_USERDEF, obj, arg, FALSE);
    w_bytes(arg, RSTRING_PTR(v), (size_t)RSTRING_LEN(v));
    if (hasiv) {
      arg->userdefs.insert((uintptr_t)mrb_ptr(obj));
      w_ivar(mrb, hasiv, ivobj, &c_arg);
      arg->userdefs.erase((uintptr_t)mrb_ptr(obj));
    }
    w_remember(mrb, obj, arg);
    return;
  }

  w_remember(mrb, obj, arg);

  hasiv = has_ivars(mrb, obj, &ivobj);
  if (hasiv) w_byte(arg, TYPE_IVAR);

  switch (mrb_type(obj)) {
    case MRB_TT_CLASS: {
      mrb_value path = class2path(mrb, (struct RClass *)mrb_ptr(obj));
      w_byte(arg, TYPE_CLASS);
      w_bytes(arg, RSTRING_PTR(path), (size_t)RSTRING_LEN(path));
      break;
    }

    case MRB_TT_MODULE: {
      mrb_value path = class2path(mrb, (struct RClass *)mrb_ptr(obj));
      w_byte(arg, TYPE_MODULE);
      w_bytes(arg, RSTRING_PTR(path), (size_t)RSTRING_LEN(path));
      break;
    }

    case MRB_TT_SCLASS:
      mrb_raise(mrb, E_TYPE_ERROR, "singleton class can't be dumped");
      break;

    case MRB_TT_INTEGER: {
      mrb_int x = mrb_integer(obj);
      if (x >> 30 == 0 || x >> 30 == -1) {
        w_byte(arg, TYPE_FIXNUM);
        w_long(x, arg);
      }
      else {
        w_bigfixnum(mrb, obj, arg);
      }
      break;
    }

    case MRB_TT_FLOAT:
      w_byte(arg, TYPE_FLOAT);
      w_float(mrb, mrb_float(obj), arg);
      break;

    case MRB_TT_STRING: {
      const char *sp = RSTRING_PTR(obj);
      bool utf8_text = str_valid_utf8(sp, RSTRING_LEN(obj));
      if (utf8_text) {
        for (mrb_int i = 0; i < RSTRING_LEN(obj); i++) {
          if ((unsigned char)sp[i] >= 0x80) {
            w_byte(arg, TYPE_IVAR);
            break;
          }
        }
      }
      w_uclass(mrb, obj, mrb->string_class, arg);
      w_byte(arg, TYPE_STRING);
      w_bytes(arg, RSTRING_PTR(obj), (size_t)RSTRING_LEN(obj));
      if (utf8_text) {
        for (mrb_int i = 0; i < RSTRING_LEN(obj); i++) {
          if ((unsigned char)sp[i] >= 0x80) {
            w_long(1, arg);
            w_symbol(mrb, mrb_intern_lit(mrb, "E"), arg);
            w_object(mrb, mrb_true_value(), arg, limit);
            break;
          }
        }
      }
      break;
    }

    case MRB_TT_ARRAY: {
      w_uclass(mrb, obj, mrb->array_class, arg);
      w_byte(arg, TYPE_ARRAY);
      mrb_int len = RARRAY_LEN(obj);
      w_long(len, arg);
      for (mrb_int i = 0; i < len; i++) {
        w_object(mrb, mrb_ary_ref(mrb, obj, i), arg, limit);
        if (len != RARRAY_LEN(obj)) {
          mrb_raise(mrb, E_RUNTIME_ERROR, "array modified during dump");
        }
      }
      break;
    }

    case MRB_TT_HASH: {
      w_uclass(mrb, obj, mrb->hash_class, arg);
      struct RHash *h = mrb_hash_ptr(obj);
      if (!(h->flags & MRB_HASH_DEFAULT)) {
        w_byte(arg, TYPE_HASH);
      }
      else if (h->flags & MRB_HASH_PROC_DEFAULT) {
        mrb_raise(mrb, E_TYPE_ERROR, "can't dump hash with default proc");
      }
      else {
        w_byte(arg, TYPE_HASH_DEF);
      }
      w_long(mrb_hash_size(mrb, obj), arg);
      mrb_hash_foreach(mrb, h, hash_each, &c_arg);
      if (h->flags & MRB_HASH_DEFAULT) {
        mrb_value dflt = mrb_iv_get(mrb, obj, mrb_intern_lit(mrb, "ifnone"));
        w_object(mrb, dflt, arg, limit);
      }
      break;
    }

    case MRB_TT_RANGE: {
      struct RRange *r = mrb_range_ptr(mrb, obj);
      w_class(mrb, TYPE_OBJECT, obj, arg, TRUE);
      w_long(3, arg);
      w_symbol(mrb, s_excl, arg);
      w_object(mrb, mrb_bool_value(mrb_range_excl_p(mrb, obj)), arg, limit);
      w_symbol(mrb, s_begin, arg);
      w_object(mrb, mrb_range_beg(mrb, obj), arg, limit);
      w_symbol(mrb, s_end, arg);
      w_object(mrb, mrb_range_end(mrb, obj), arg, limit);
      break;
    }

    case MRB_TT_STRUCT: {
      w_class(mrb, TYPE_STRUCT, obj, arg, TRUE);
      mrb_int len = RARRAY_LEN(obj);
      struct RClass *klass = mrb_class_real(mrb_class(mrb, obj));
      mrb_value mem = mrb_iv_get(mrb, mrb_obj_value(klass), mrb_intern_lit(mrb, "__members__"));
      if (!mrb_array_p(mem)) {
        mrb_raisef(mrb, E_TYPE_ERROR, "can't dump %S",
                   mrb_str_new_cstr(mrb, mrb_class_name(mrb, klass)));
      }
      w_long(len, arg);
      for (mrb_int i = 0; i < len; i++) {
        w_symbol(mrb, mrb_symbol(mrb_ary_ref(mrb, mem, i)), arg);
        w_object(mrb, mrb_ary_ref(mrb, obj, i), arg, limit);
      }
      break;
    }

    case MRB_TT_OBJECT:
    case MRB_TT_EXCEPTION:
      w_class(mrb, TYPE_OBJECT, obj, arg, TRUE);
      w_objivar(mrb, obj, &c_arg);
      break;

    case MRB_TT_CDATA: {
      struct RClass *regexp_class = mrb_class_get(mrb, "Regexp");
      if (regexp_class && mrb_obj_is_kind_of(mrb, obj, regexp_class)) {
        mrb_value src = mrb_funcall_id(mrb, obj, s_source, 0, 0);
        mrb_value opts = mrb_funcall_id(mrb, obj, s_options, 0, 0);
        if (mrb_string_p(src)) {
          w_byte(arg, TYPE_REGEXP);
          w_bytes(arg, RSTRING_PTR(src), (size_t)RSTRING_LEN(src));
          w_byte(arg, (char)(mrb_integer(opts) & 0xff));
          break;
        }
      }
      if (!mrb_respond_to(mrb, obj, s_dump_data)) {
        mrb_raisef(mrb, E_TYPE_ERROR, "no _dump_data is defined for class %S",
                   mrb_str_new_cstr(mrb, mrb_class_name(mrb, mrb_obj_class(mrb, obj))));
      }
      v = dump_funcall(mrb, arg, obj, s_dump_data, 0, 0);
      gc_roots_push(mrb, &arg->gc_roots, v);
      w_class(mrb, TYPE_DATA, obj, arg, TRUE);
      w_object(mrb, v, arg, limit);
      break;
    }

    case MRB_TT_PROC:
    case MRB_TT_FIBER: {
      if (!mrb_respond_to(mrb, obj, s_dump_data)) {
        mrb_raisef(mrb, E_TYPE_ERROR, "no _dump_data is defined for class %S",
                   mrb_str_new_cstr(mrb, mrb_class_name(mrb, mrb_obj_class(mrb, obj))));
      }
      v = dump_funcall(mrb, arg, obj, s_dump_data, 0, 0);
      gc_roots_push(mrb, &arg->gc_roots, v);
      w_class(mrb, TYPE_DATA, obj, arg, TRUE);
      w_object(mrb, v, arg, limit);
      break;
    }

    default:
      mrb_raisef(mrb, E_TYPE_ERROR, "can't dump %S",
                 mrb_str_new_cstr(mrb, mrb_class_name(mrb, mrb_obj_class(mrb, obj))));
      break;
  }

  if (hasiv) {
    w_ivar(mrb, hasiv, ivobj, &c_arg);
  }
}

// ---------------------------------------------------------------------------
// Marshal.dump
// ---------------------------------------------------------------------------

static void io_needed(mrb_state *mrb)
{
  mrb_raise(mrb, E_TYPE_ERROR, "instance of IO needed");
}

static mrb_int num2int(mrb_state *mrb, mrb_value v)
{
  if (mrb_fixnum_p(v)) return mrb_fixnum(v);
  if (mrb_float_p(v)) return (mrb_int)mrb_float(v);
  mrb_raise(mrb, E_TYPE_ERROR, "no implicit conversion to Integer");
}

struct dump_context {
  dump_arg arg;
  mrb_value obj;
  mrb_int limit;
};

static mrb_value marshal_dump_body(mrb_state *mrb, void *userdata)
{
  dump_context *ctx = (dump_context *)userdata;

  w_byte(&ctx->arg, MARSHAL_MAJOR);
  w_byte(&ctx->arg, MARSHAL_MINOR);
  w_object(mrb, ctx->obj, &ctx->arg, ctx->limit);
  if (!mrb_nil_p(ctx->arg.dest)) {
    mrb_value args[1] = { ctx->arg.str };
    mrb_funcall_argv(mrb, ctx->arg.dest, s_write, 1, args);
  }
  return mrb_nil_value();
}

mrb_value mrb_marshal_dump_limited(mrb_state *mrb, mrb_value obj, mrb_value port, mrb_int limit)
{
  dump_context ctx;
  ctx.arg.mrb = mrb;
  ctx.arg.active = true;
  ctx.arg.dest = mrb_nil_value();
  ctx.arg.str = mrb_str_new(mrb, NULL, 0);
  gc_roots_push(mrb, &ctx.arg.gc_roots, ctx.arg.str);
  ctx.obj = obj;
  ctx.limit = limit;

  if (!mrb_nil_p(port)) {
    if (!mrb_respond_to(mrb, port, s_write)) {
      clear_dump_arg(&ctx.arg);
      io_needed(mrb);
    }
    gc_roots_push(mrb, &ctx.arg.gc_roots, port);
    ctx.arg.dest = port;
    if (mrb_respond_to(mrb, port, s_binmode)) {
      mrb_funcall_argv(mrb, port, s_binmode, 0, NULL);
    }
  }

  mrb_bool err = FALSE;
  mrb_value result = mrb_protect_error(mrb, marshal_dump_body, &ctx, &err);
  clear_dump_arg(&ctx.arg);
  if (err) {
    mrb_exc_raise(mrb, result);
  }
  return mrb_nil_p(port) ? ctx.arg.str : port;
}

static mrb_value marshal_dump(mrb_state *mrb, mrb_value mod)
{
  mrb_value obj, a1, a2, port = mrb_nil_value();
  mrb_int limit = -1;
  mrb_int argc = mrb_get_argc(mrb);

  mrb_get_args(mrb, "o|oo", &obj, &a1, &a2);
  if (argc == 3) {
    if (!mrb_nil_p(a2)) limit = num2int(mrb, a2);
    if (mrb_nil_p(a1)) io_needed(mrb);
    port = a1;
  }
  else if (argc == 2) {
    if (mrb_fixnum_p(a1)) limit = mrb_fixnum(a1);
    else if (mrb_nil_p(a1)) io_needed(mrb);
    else port = a1;
  }
  return mrb_marshal_dump_limited(mrb, obj, port, limit);
}

// ---------------------------------------------------------------------------
// Load-side helpers
// ---------------------------------------------------------------------------

static mrb_value check_load_arg(mrb_state *mrb, mrb_value ret, load_arg *arg, const char *name)
{
  if (!arg->active) {
    mrb_raisef(mrb, E_RUNTIME_ERROR, "Marshal.load reentered at %s", name);
  }
  return ret;
}

#define load_funcall(mrb, arg, obj, sym, argc, argv) \
  check_load_arg(mrb, mrb_funcall_argv(mrb, obj, sym, argc, argv), arg, mrb_sym_name(mrb, sym))

static void too_short(mrb_state *mrb)
{
  mrb_raise(mrb, E_ARGUMENT_ERROR, "marshal data too short");
}

static struct RClass *eof_error(mrb_state *mrb)
{
  if (mrb_class_defined(mrb, "EOFError")) {
    return mrb_class_get(mrb, "EOFError");
  }
  return mrb_exc_get_id(mrb, MRB_ERROR_SYM(IndexError));
}

static mrb_int r_byte(mrb_state *mrb, load_arg *arg)
{
  if (!arg->is_io) {
    if (RSTRING_LEN(arg->src) > arg->offset) {
      return (unsigned char)RSTRING_PTR(arg->src)[arg->offset++];
    }
    too_short(mrb);
  }
  mrb_value v = load_funcall(mrb, arg, arg->src, s_getbyte, 0, 0);
  if (mrb_nil_p(v)) {
    mrb_raise(mrb, eof_error(mrb), "end of file reached");
  }
  return mrb_integer(v);
}

static void long_toobig(mrb_state *mrb, mrb_int size)
{
  mrb_raisef(mrb, E_TYPE_ERROR, "long too big for this architecture (size %d, given %d)",
             (int)sizeof(mrb_int), (int)size);
}

static mrb_int r_long(mrb_state *mrb, load_arg *arg)
{
  mrb_int x;
  mrb_int c = (signed char)r_byte(mrb, arg);
  mrb_int i;

  if (c == 0) return 0;
  if (c > 0) {
    if (4 < c && c < 128) {
      return c - 5;
    }
    if (c > (mrb_int)sizeof(mrb_int)) long_toobig(mrb, c);
    x = 0;
    for (i = 0; i < c; i++) {
      x |= (mrb_int)r_byte(mrb, arg) << (8 * i);
    }
  }
  else {
    if (-129 < c && c < -4) {
      return c + 5;
    }
    c = -c;
    if (c > (mrb_int)sizeof(mrb_int)) long_toobig(mrb, c);
    x = -1;
    for (i = 0; i < c; i++) {
      x &= ~((mrb_int)0xff << (8 * i));
      x |= (mrb_int)r_byte(mrb, arg) << (8 * i);
    }
  }
  return x;
}

static mrb_value r_bytes(mrb_state *mrb, load_arg *arg)
{
  mrb_int len = r_long(mrb, arg);
  if (len == 0) {
    return mrb_str_new(mrb, NULL, 0);
  }
  if (!arg->is_io) {
    if (RSTRING_LEN(arg->src) - arg->offset >= len) {
      mrb_value s = mrb_str_new(mrb, RSTRING_PTR(arg->src) + arg->offset, len);
      arg->offset += len;
      return s;
    }
    too_short(mrb);
  }
  if (len < 0) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "negative length");
  }
  mrb_value n = mrb_fixnum_value(len);
  mrb_value str = load_funcall(mrb, arg, arg->src, s_read, 1, &n);
  if (mrb_nil_p(str) || !mrb_string_p(str) || RSTRING_LEN(str) != len) {
    too_short(mrb);
  }
  return str;
}

static mrb_value r_string(mrb_state *mrb, load_arg *arg)
{
  return r_bytes(mrb, arg);
}

static mrb_value r_symlink(mrb_state *mrb, load_arg *arg)
{
  mrb_int num = r_long(mrb, arg);
  auto it = arg->symbols.find(num);
  if (it == arg->symbols.end()) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "bad symbol");
  }
  return it->second;
}

static mrb_value r_symbol(mrb_state *mrb, load_arg *arg);
static mrb_value r_object(mrb_state *mrb, load_arg *arg);

static mrb_value r_symreal(mrb_state *mrb, load_arg *arg, bool ivar)
{
  mrb_value s = r_bytes(mrb, arg);
  mrb_int n = (mrb_int)arg->symbols.size();
  gc_roots_push(mrb, &arg->gc_roots, s);
  arg->symbols[n] = s;
  if (ivar) {
    mrb_int num = r_long(mrb, arg);
    while (num-- > 0) {
      mrb_value sym = r_symbol(mrb, arg);
      mrb_value val = r_object(mrb, arg);
      (void)sym;
      (void)val;
      // encoding ivars are ignored (mruby has no encodings)
    }
  }
  return s;
}

static mrb_value r_symbol(mrb_state *mrb, load_arg *arg)
{
  mrb_int type;
  bool ivar = false;

  for (;;) {
    switch ((type = r_byte(mrb, arg))) {
      default:
        mrb_raisef(mrb, E_ARGUMENT_ERROR, "dump format error for symbol(0x%u)", (unsigned)type);
      case TYPE_IVAR:
        ivar = true;
        continue;
      case TYPE_SYMBOL:
        return r_symreal(mrb, arg, ivar);
      case TYPE_SYMLINK:
        if (ivar) {
          mrb_raise(mrb, E_ARGUMENT_ERROR, "dump format error (symlink with encoding)");
        }
        return r_symlink(mrb, arg);
    }
  }
}

static mrb_value r_unique(mrb_state *mrb, load_arg *arg)
{
  return r_symbol(mrb, arg);
}

static mrb_value r_entry0(mrb_state *mrb, mrb_value v, mrb_int num, load_arg *arg)
{
  if (num == (mrb_int)arg->data.size()) {
    arg->data.push_back(v);
  }
  else {
    arg->data[num] = v;
  }
  gc_roots_push(mrb, &arg->gc_roots, v);
  arg->partial_objects.insert(mrb_basic_ptr(v));
  return v;
}

static mrb_value r_entry(mrb_state *mrb, mrb_value v, load_arg *arg)
{
  return r_entry0(mrb, v, (mrb_int)arg->data.size(), arg);
}

static mrb_int r_prepare(mrb_state *mrb, load_arg *arg)
{
  mrb_int idx = (mrb_int)arg->data.size();
  arg->data.push_back(mrb_undef_value());
  return idx;
}

static mrb_value r_post_proc(mrb_state *mrb, mrb_value v, load_arg *arg)
{
  if (!mrb_nil_p(arg->proc)) {
    mrb_value args[1] = { v };
    v = load_funcall(mrb, arg, arg->proc, s_call, 1, args);
  }
  return v;
}

static mrb_value r_leave(mrb_state *mrb, mrb_value v, load_arg *arg, bool partial)
{
  if (!partial) {
    if (!mrb_immediate_p(v)) {
      arg->partial_objects.erase(mrb_basic_ptr(v));
    }
    v = r_post_proc(mrb, v, arg);
  }
  return v;
}

static bool is_enc_name_string(mrb_value sym)
{
  mrb_int len = RSTRING_LEN(sym);
  const char *p = RSTRING_PTR(sym);
  return (len == 1 && p[0] == 'E') ||
         (len == 8 && memcmp(p, "encoding", 8) == 0);
}

static void override_ivar_error(mrb_state *mrb, const char *type, mrb_value str)
{
  mrb_raisef(mrb, E_TYPE_ERROR, "can't override instance variable of %s '%S'", type, str);
}

static bool r_encname(mrb_state *mrb, load_arg *arg)
{
  mrb_int len = r_long(mrb, arg);
  if (len > 0) {
    mrb_value sym = r_symbol(mrb, arg);
    mrb_value val = r_object(mrb, arg);
    (void)val;
    if (is_enc_name_string(sym)) {
      len--;
    }
  }
  return len > 0;
}

// Types that can hold instance variables. Others (String/Array/Float/Range)
// silently discard incoming ivars on load.
static bool iv_capable_p(enum mrb_vtype tt)
{
  switch (tt) {
    case MRB_TT_OBJECT:
    case MRB_TT_CLASS:
    case MRB_TT_MODULE:
    case MRB_TT_SCLASS:
    case MRB_TT_HASH:
    case MRB_TT_CDATA:
    case MRB_TT_EXCEPTION:
      return true;
    default:
      return false;
  }
}

static void r_ivar(mrb_state *mrb, mrb_value obj, load_arg *arg, bool *has_encoding)
{
  mrb_int len = r_long(mrb, arg);
  if (len > 0) {
    enum mrb_vtype tt = mrb_type(obj);
    bool is_class = (tt == MRB_TT_CLASS || tt == MRB_TT_MODULE);
    bool can_hold = !mrb_immediate_p(obj) && iv_capable_p(tt);
    do {
      mrb_value sym = r_symbol(mrb, arg);
      mrb_value val = r_object(mrb, arg);
      if (is_enc_name_string(sym)) {
        if (has_encoding) *has_encoding = true;
      }
      else if (RSTRING_LEN(sym) == 1 && RSTRING_PTR(sym)[0] == 'K') {
        // ruby2_keywords flag: no such concept in mruby
        if (tt != MRB_TT_HASH) {
          mrb_raise(mrb, E_ARGUMENT_ERROR,
                    "ruby2_keywords flag is given but the object is not a Hash");
        }
      }
      else if (can_hold) {
        mrb_sym id = mrb_intern_str(mrb, sym);
        if (is_class) {
          mrb_int nlen;
          const char *name = mrb_sym_name_len(mrb, id, &nlen);
          if (nlen == 0 || name[0] != '@') {
            override_ivar_error(mrb, tt == MRB_TT_MODULE ? "module" : "class",
                                mrb_obj_value(mrb_class_ptr(obj)));
          }
        }
        mrb_obj_iv_set(mrb, mrb_obj_ptr(obj), id, val);
      }
      // else: type cannot hold ivars in mruby; consume and discard the pair.
      // TYPE_USERDEF payload String ivars are private to _dump; _load rebuilds
      // from the payload bytes alone.
    } while (--len > 0);
  }
}

static int copy_ivar_i(mrb_state *mrb, mrb_sym id, mrb_value value, void *p)
{
  mrb_value obj = mrb_obj_value(p);
  if (!mrb_iv_defined(mrb, obj, id)) {
    mrb_obj_iv_set(mrb, mrb_obj_ptr(obj), id, value);
  }
  return 0;
}

static mrb_value r_copy_ivar(mrb_state *mrb, mrb_value v, mrb_value data)
{
  mrb_iv_foreach(mrb, data, copy_ivar_i, mrb_ptr(v));
  return v;
}

// ---------------------------------------------------------------------------
// Resolves "A::B" style paths segment by segment. Missing classes raise
// NameError (no auto-creation, matching CRuby 4.0.6).
// ---------------------------------------------------------------------------

static mrb_value path_to_class(mrb_state *mrb, mrb_value path)
{
  const char *p = RSTRING_PTR(path);
  const char *end = p + RSTRING_LEN(path);
  mrb_value mod = mrb_obj_value(mrb->object_class);

  while (p < end) {
    const char *q = p;
    while (q < end && !(q[0] == ':' && q + 1 < end && q[1] == ':')) q++;
    if (q > p) {
      mrb_sym sym = mrb_intern(mrb, p, (mrb_int)(q - p));
      if (!mrb_const_defined(mrb, mod, sym)) {
        mrb_raisef(mrb, E_NAME_ERROR, "uninitialized constant %S", path);
      }
      mod = mrb_const_get(mrb, mod, sym);
    }
    p = q + 2;
  }
  return mod;
}

static mrb_value path2class(mrb_state *mrb, mrb_value path)
{
  mrb_value v = path_to_class(mrb, path);
  if (mrb_type(v) != MRB_TT_CLASS) {
    mrb_raisef(mrb, E_ARGUMENT_ERROR, "%S does not refer to class", path);
  }
  return v;
}

static mrb_value path2module(mrb_state *mrb, mrb_value path)
{
  mrb_value v = path_to_class(mrb, path);
  if (mrb_type(v) != MRB_TT_MODULE) {
    mrb_raisef(mrb, E_ARGUMENT_ERROR, "%S does not refer to module", path);
  }
  return v;
}

static mrb_value obj_alloc(mrb_state *mrb, mrb_value klass)
{
  return mrb_obj_value(mrb_obj_alloc(mrb, MRB_TT_OBJECT, mrb_class_ptr(klass)));
}

#define prohibit_ivar(mrb, type, str, ivp) \
  do { \
    if (!(ivp) || !*(ivp)) break; \
    override_ivar_error(mrb, type, str); \
  } while (0)

static mrb_value r_object_for(mrb_state *mrb, load_arg *arg, bool partial,
                              bool *ivp, std::vector<mrb_value> *extmod, mrb_int type);

static mrb_value r_object0(mrb_state *mrb, load_arg *arg, bool partial,
                           bool *ivp, std::vector<mrb_value> *extmod)
{
  mrb_int type = r_byte(mrb, arg);
  return r_object_for(mrb, arg, partial, ivp, extmod, type);
}

static mrb_value r_object_for(mrb_state *mrb, load_arg *arg, bool partial,
                              bool *ivp, std::vector<mrb_value> *extmod, mrb_int type)
{
  mrb_value v = mrb_nil_value();

  switch (type) {
    case TYPE_LINK: {
      mrb_int id = r_long(mrb, arg);
      if (id < 0 || id >= (mrb_int)arg->data.size()) {
        mrb_raise(mrb, E_ARGUMENT_ERROR, "dump format error (unlinked)");
      }
      v = arg->data[id];
      if (!mrb_undef_p(v) &&
          arg->partial_objects.count(mrb_basic_ptr(v)) == 0) {
        v = r_post_proc(mrb, v, arg);
      }
      break;
    }

    case TYPE_IVAR: {
      bool ivar = true;
      v = r_object0(mrb, arg, true, &ivar, extmod);
      if (ivar) r_ivar(mrb, v, arg, NULL);
      v = r_leave(mrb, v, arg, partial);
      break;
    }

    case TYPE_EXTENDED: {
      mrb_value path = r_unique(mrb, arg);
      mrb_value m = path_to_class(mrb, path);
      if (mrb_type(m) == MRB_TT_CLASS) {  // prepended
        v = r_object0(mrb, arg, true, NULL, NULL);
        struct RClass *c = mrb_class(mrb, v);
        if (c != mrb_class_ptr(m) || mrb_type(mrb_obj_value(c)) == MRB_TT_SCLASS) {
          mrb_raisef(mrb, E_ARGUMENT_ERROR,
                     "prepended class %S differs from class %S", path,
                     mrb_obj_value(c));
        }
        if (extmod) {
          for (auto it = extmod->rbegin(); it != extmod->rend(); ++it) {
            mrb_value args[1] = { *it };
            load_funcall(mrb, arg, v, s_prepend, 1, args);
          }
        }
      }
      else {
        if (mrb_type(m) != MRB_TT_MODULE) {
          mrb_raisef(mrb, E_ARGUMENT_ERROR, "%S does not refer to module", path);
        }
        std::vector<mrb_value> local_extmod;
        if (!extmod) extmod = &local_extmod;
        extmod->push_back(m);
        v = r_object0(mrb, arg, true, NULL, extmod);
        for (auto it = extmod->rbegin(); it != extmod->rend(); ++it) {
          mrb_value args[1] = { *it };
          load_funcall(mrb, arg, v, s_extend, 1, args);
        }
        extmod->clear();
      }
      v = r_leave(mrb, v, arg, partial);
      break;
    }

    case TYPE_UCLASS: {
      mrb_value c = path2class(mrb, r_unique(mrb, arg));
      if (mrb_type(c) == MRB_TT_SCLASS) {
        mrb_raise(mrb, E_TYPE_ERROR, "singleton can't be loaded");
      }
      type = r_byte(mrb, arg);
      v = r_object_for(mrb, arg, partial, NULL, extmod, type);
      if (mrb_immediate_p(v) || mrb_type(v) == MRB_TT_OBJECT ||
          mrb_type(v) == MRB_TT_CLASS) {
        mrb_raise(mrb, E_ARGUMENT_ERROR, "dump format error (user class)");
      }
      if (mrb_type(v) != MRB_TT_MODULE) {
        bool ancestor = false;
        for (struct RClass *k = mrb_class(mrb, v); k; k = k->super) {
          if (k == mrb_class_ptr(c)) {
            ancestor = true;
            break;
          }
        }
        if (!ancestor) {
          mrb_value tmp = obj_alloc(mrb, c);
          if (mrb_type(tmp) != mrb_type(v)) {
            mrb_raise(mrb, E_ARGUMENT_ERROR, "dump format error (user class)");
          }
        }
      }
      mrb_basic_ptr(v)->c = mrb_class_ptr(c);
      break;
    }

    case TYPE_NIL:
      v = mrb_nil_value();
      v = r_leave(mrb, v, arg, false);
      break;

    case TYPE_TRUE:
      v = mrb_true_value();
      v = r_leave(mrb, v, arg, false);
      break;

    case TYPE_FALSE:
      v = mrb_false_value();
      v = r_leave(mrb, v, arg, false);
      break;

    case TYPE_FIXNUM: {
      mrb_int i = r_long(mrb, arg);
      v = mrb_fixnum_value(i);
      v = r_leave(mrb, v, arg, false);
      break;
    }

    case TYPE_FLOAT: {
      mrb_value str = r_bytes(mrb, arg);
      const char *ptr = RSTRING_PTR(str);
      double d;
      if (strcmp(ptr, "nan") == 0) {
        d = NAN;
      }
      else if (strcmp(ptr, "inf") == 0) {
        d = INFINITY;
      }
      else if (strcmp(ptr, "-inf") == 0) {
        d = -INFINITY;
      }
      else {
        char *e;
        d = strtod(ptr, &e);
        d = load_mantissa(d, e, RSTRING_LEN(str) - (e - ptr));
      }
      v = mrb_float_value(mrb, d);
      v = r_entry(mrb, v, arg);
      v = r_leave(mrb, v, arg, false);
      break;
    }

    case TYPE_BIGNUM: {
      mrb_int sign = r_byte(mrb, arg);
      if (sign != '+' && sign != '-') {
        mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid Bignum sign");
      }
      mrb_int len = r_long(mrb, arg);
      if (len < 0) {
        mrb_raise(mrb, E_ARGUMENT_ERROR, "negative length");
      }
      if (len > 4) {
        mrb_raise(mrb, E_RANGE_ERROR, "integer too big");  // no bignums in mruby
      }
      mrb_uint num = 0;
      for (mrb_int i = 0; i < len; i++) {
        num |= (mrb_uint)r_byte(mrb, arg) << (i * 16);
        num |= (mrb_uint)r_byte(mrb, arg) << (i * 16 + 8);
      }
      if (sign == '-') {
        if (num > ((mrb_uint)1 << 63)) {
          mrb_raise(mrb, E_RANGE_ERROR, "integer too big");
        }
        v = mrb_int_value(mrb, (mrb_int)-num);
      }
      else {
        if (num > (mrb_uint)MRB_INT_MAX) {
          mrb_raise(mrb, E_RANGE_ERROR, "integer too big");
        }
        v = mrb_int_value(mrb, (mrb_int)num);
      }
      v = r_entry(mrb, v, arg);
      v = r_leave(mrb, v, arg, false);
      break;
    }

    case TYPE_STRING:
      v = r_entry(mrb, r_string(mrb, arg), arg);
      v = r_leave(mrb, v, arg, partial);
      break;

    case TYPE_REGEXP: {
      mrb_value str = r_bytes(mrb, arg);
      mrb_int options = r_byte(mrb, arg);
      mrb_int idx = r_prepare(mrb, arg);

      if (ivp) {
        r_ivar(mrb, str, arg, NULL);
        *ivp = false;
      }
      // 1.8 compatibility: drop escapes undefined in 1.8 (ported from 4.0.6)
      {
        char *ptr = RSTRING_PTR(str);
        mrb_int len = RSTRING_LEN(str);
        mrb_int bs = 0;
        char *dst = ptr;
        char *src = ptr;
        for (; len-- > 0; *dst++ = *src++) {
          switch (*src) {
            case '\\': bs++; break;
            case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
            case 'm': case 'o': case 'p': case 'q': case 'u': case 'y':
            case 'E': case 'F': case 'H': case 'I': case 'J': case 'K':
            case 'L': case 'N': case 'O': case 'P': case 'Q': case 'R':
            case 'S': case 'T': case 'U': case 'V': case 'X': case 'Y':
              if (bs & 1) dst--;
              /* fall through */
            default: bs = 0; break;
          }
        }
        mrb_str_resize(mrb, str, dst - ptr);
      }
      mrb_value regexp_class = mrb_obj_value(mrb->object_class);
      regexp_class = mrb_const_get(mrb, mrb_obj_value(mrb->object_class),
                                   mrb_intern_lit(mrb, "Regexp"));
      mrb_value args[2] = { str, mrb_fixnum_value(options) };
      mrb_value regexp = mrb_funcall_argv(mrb, regexp_class, s_new, 2, args);
      r_copy_ivar(mrb, regexp, str);

      v = r_entry0(mrb, regexp, idx, arg);
      v = r_leave(mrb, v, arg, partial);
      break;
    }

    case TYPE_ARRAY: {
      mrb_int len = r_long(mrb, arg);
      if (len < 0) {
        mrb_raise(mrb, E_ARGUMENT_ERROR, "negative length");
      }
      v = mrb_ary_new_capa(mrb, len);
      v = r_entry(mrb, v, arg);
      for (mrb_int i = 0; i < len; i++) {
        mrb_ary_push(mrb, v, r_object(mrb, arg));
      }
      v = r_leave(mrb, v, arg, partial);
      break;
    }

    case TYPE_HASH:
    case TYPE_HASH_DEF: {
      mrb_int len = r_long(mrb, arg);
      if (len < 0) {
        mrb_raise(mrb, E_ARGUMENT_ERROR, "negative length");
      }
      v = mrb_hash_new_capa(mrb, len);
      v = r_entry(mrb, v, arg);
      for (mrb_int i = 0; i < len; i++) {
        mrb_value key = r_object(mrb, arg);
        mrb_value value = r_object(mrb, arg);
        mrb_hash_set(mrb, v, key, value);
      }
      if (type == TYPE_HASH_DEF) {
        mrb_value dflt = r_object(mrb, arg);
        struct RHash *h = mrb_hash_ptr(v);
        h->flags |= MRB_HASH_DEFAULT;
        mrb_iv_set(mrb, v, mrb_intern_lit(mrb, "ifnone"), dflt);
      }
      v = r_leave(mrb, v, arg, partial);
      break;
    }

    case TYPE_STRUCT: {
      mrb_int idx = r_prepare(mrb, arg);
      mrb_value klass_v = path2class(mrb, r_unique(mrb, arg));
      struct RClass *klass = mrb_class_ptr(klass_v);
      mrb_int len = r_long(mrb, arg);
      if (len < 0) {
        mrb_raise(mrb, E_ARGUMENT_ERROR, "negative length");
      }

      v = obj_alloc(mrb, klass_v);
      if (mrb_type(v) != MRB_TT_STRUCT) {
        mrb_raisef(mrb, E_TYPE_ERROR, "class %s not a struct",
                   mrb_class_name(mrb, klass));
      }
      mrb_value mem = mrb_iv_get(mrb, klass_v, mrb_intern_lit(mrb, "__members__"));
      if (!mrb_array_p(mem) || RARRAY_LEN(mem) != len) {
        mrb_raisef(mrb, E_TYPE_ERROR, "struct %s not compatible (struct size differs)",
                   mrb_class_name(mrb, klass));
      }
      v = r_entry0(mrb, v, idx, arg);
      for (mrb_int i = 0; i < len; i++) {
        mrb_value n = mrb_ary_ref(mrb, mem, i);
        mrb_value slot = r_symbol(mrb, arg);
        mrb_int nl, sl;
        const char *nn = mrb_sym_name_len(mrb, mrb_symbol(n), &nl);
        const char *sn = RSTRING_PTR(slot);
        sl = RSTRING_LEN(slot);
        if (nl != sl || memcmp(nn, sn, (size_t)nl) != 0) {
          mrb_raisef(mrb, E_TYPE_ERROR,
                     "struct %s not compatible (:%S for :%S)",
                     mrb_class_name(mrb, klass), slot, mrb_symbol_value(mrb_intern(mrb, nn, nl)));
        }
        mrb_ary_set(mrb, v, i, r_object(mrb, arg));
      }
      v = r_leave(mrb, v, arg, partial);
      break;
    }

    case TYPE_USERDEF: {
      mrb_value name = r_unique(mrb, arg);
      mrb_value klass_v = path2class(mrb, name);
      if (!mrb_respond_to(mrb, klass_v, s_load)) {
        mrb_raisef(mrb, E_TYPE_ERROR, "class %S needs to have method '_load'", name);
      }
      mrb_value data = r_string(mrb, arg);
      if (ivp) {
        r_ivar(mrb, data, arg, NULL);
        *ivp = false;
      }
      v = load_funcall(mrb, arg, klass_v, s_load, 1, &data);
      v = r_entry(mrb, v, arg);
      if (!partial) {
        v = r_post_proc(mrb, v, arg);
      }
      break;
    }

    case TYPE_USRMARSHAL: {
      mrb_value name = r_unique(mrb, arg);
      mrb_value klass_v = path2class(mrb, name);

      v = obj_alloc(mrb, klass_v);
      if (extmod) {
        for (auto it = extmod->rbegin(); it != extmod->rend(); ++it) {
          mrb_value args[1] = { *it };
          load_funcall(mrb, arg, v, s_extend, 1, args);
        }
      }
      if (!mrb_respond_to(mrb, v, s_mload)) {
        mrb_raisef(mrb, E_TYPE_ERROR,
                   "instance of %S needs to have method 'marshal_load'", name);
      }
      v = r_entry(mrb, v, arg);
      mrb_value data = r_object(mrb, arg);
      load_funcall(mrb, arg, v, s_mload, 1, &data);
      v = r_copy_ivar(mrb, v, data);
      if (!partial) {
        v = r_post_proc(mrb, v, arg);
      }
      if (extmod) extmod->clear();
      break;
    }

    case TYPE_OBJECT: {
      mrb_int idx = r_prepare(mrb, arg);
      mrb_value klass_v = path2class(mrb, r_unique(mrb, arg));
      struct RClass *klass = mrb_class_ptr(klass_v);

      if (klass == mrb->range_class) {
        // 4.0.6 dumps Ranges as TYPE_OBJECT with "excl"/"begin"/"end"
        // ivars; rebuild the mruby Range struct from them.
        mrb_int count = r_long(mrb, arg);
        mrb_value excl = mrb_undef_value();
        mrb_value beg = mrb_undef_value();
        mrb_value end = mrb_undef_value();
        for (mrb_int i = 0; i < count; i++) {
          mrb_value sym = r_symbol(mrb, arg);
          mrb_value val = r_object(mrb, arg);
          mrb_int l = RSTRING_LEN(sym);
          const char *p = RSTRING_PTR(sym);
          if (l == 4 && memcmp(p, "excl", 4) == 0) excl = val;
          else if (l == 5 && memcmp(p, "begin", 5) == 0) beg = val;
          else if (l == 3 && memcmp(p, "end", 3) == 0) end = val;
        }
        if (mrb_undef_p(excl) || mrb_undef_p(beg) || mrb_undef_p(end)) {
          mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid Range");
        }
        v = mrb_range_new(mrb, beg, end, mrb_test(excl));
        v = r_entry0(mrb, v, idx, arg);
      }
      else {
        v = obj_alloc(mrb, klass_v);
        if (mrb_type(v) != MRB_TT_OBJECT && mrb_type(v) != MRB_TT_EXCEPTION) {
          mrb_raise(mrb, E_ARGUMENT_ERROR, "dump format error");
        }
        v = r_entry0(mrb, v, idx, arg);
        r_ivar(mrb, v, arg, NULL);
      }
      v = r_leave(mrb, v, arg, partial);
      break;
    }

    case TYPE_DATA: {
      mrb_value name = r_unique(mrb, arg);
      mrb_value klass_v = path2class(mrb, name);

      v = obj_alloc(mrb, klass_v);
      if (mrb_type(v) != MRB_TT_CDATA) {
        mrb_raise(mrb, E_ARGUMENT_ERROR, "dump format error");
      }
      v = r_entry(mrb, v, arg);
      if (!mrb_respond_to(mrb, v, s_load_data)) {
        mrb_raisef(mrb, E_TYPE_ERROR,
                   "class %S needs to have instance method '_load_data'", name);
      }
      mrb_value r = r_object0(mrb, arg, partial, NULL, extmod);
      load_funcall(mrb, arg, v, s_load_data, 1, &r);
      v = r_leave(mrb, v, arg, partial);
      break;
    }

    case TYPE_MODULE_OLD: {
      mrb_value str = r_bytes(mrb, arg);
      v = path_to_class(mrb, str);
      prohibit_ivar(mrb, "class/module", str, ivp);
      v = r_entry(mrb, v, arg);
      v = r_leave(mrb, v, arg, partial);
      break;
    }

    case TYPE_CLASS: {
      mrb_value str = r_bytes(mrb, arg);
      if (ivp && *ivp) {
        *ivp = r_encname(mrb, arg);
      }
      v = path2class(mrb, str);
      prohibit_ivar(mrb, "class", str, ivp);
      v = r_entry(mrb, v, arg);
      v = r_leave(mrb, v, arg, partial);
      break;
    }

    case TYPE_MODULE: {
      mrb_value str = r_bytes(mrb, arg);
      if (ivp && *ivp) {
        *ivp = r_encname(mrb, arg);
      }
      v = path2module(mrb, str);
      prohibit_ivar(mrb, "module", str, ivp);
      v = r_entry(mrb, v, arg);
      v = r_leave(mrb, v, arg, partial);
      break;
    }

    case TYPE_SYMBOL: {
      mrb_value s;
      if (ivp) {
        s = r_symreal(mrb, arg, *ivp);
        *ivp = false;
      }
      else {
        s = r_symreal(mrb, arg, false);
      }
      v = mrb_symbol_value(mrb_intern_str(mrb, s));
      v = r_leave(mrb, v, arg, partial);
      break;
    }

    case TYPE_SYMLINK:
      v = mrb_symbol_value(mrb_intern_str(mrb, r_symlink(mrb, arg)));
      break;

    default:
      mrb_raisef(mrb, E_ARGUMENT_ERROR, "dump format error(0x%x)", type);
      break;
  }

  if (mrb_undef_p(v)) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "dump format error (bad link)");
  }
  return v;
}

static mrb_value r_object(mrb_state *mrb, load_arg *arg)
{
  return r_object0(mrb, arg, false, NULL, NULL);
}

// ---------------------------------------------------------------------------
// Marshal.load
// ---------------------------------------------------------------------------

static mrb_value string_value(mrb_state *mrb, mrb_value port)
{
  if (mrb_string_p(port)) return port;
  if (mrb_respond_to(mrb, port, s_to_str)) {
    return mrb_funcall_argv(mrb, port, s_to_str, 0, NULL);
  }
  return mrb_nil_value();
}

struct load_context {
  load_arg arg;
  mrb_value result;
};

static mrb_value marshal_load_body(mrb_state *mrb, void *userdata)
{
  load_context *ctx = (load_context *)userdata;
  load_arg *arg = &ctx->arg;

  mrb_int major = r_byte(mrb, arg);
  mrb_int minor = r_byte(mrb, arg);
  if (major != MARSHAL_MAJOR || minor > MARSHAL_MINOR) {
    mrb_raisef(mrb, E_TYPE_ERROR,
               "incompatible marshal file format (can't be read)\n"
               "\tformat version %d.%d required; %d.%d given",
               MARSHAL_MAJOR, MARSHAL_MINOR, major, minor);
  }
  mrb_value v = r_object(mrb, arg);
  gc_roots_push(mrb, &arg->gc_roots, v);
  ctx->result = v;
  return v;
}

mrb_value mrb_marshal_load_with_proc(mrb_state *mrb, mrb_value port, mrb_value proc)
{
  mrb_value str = string_value(mrb, port);
  if (mrb_nil_p(str)) {
    if (mrb_respond_to(mrb, port, s_getbyte) && mrb_respond_to(mrb, port, s_read)) {
      if (mrb_respond_to(mrb, port, s_binmode)) {
        mrb_funcall_argv(mrb, port, s_binmode, 0, NULL);
      }
    }
    else {
      io_needed(mrb);
    }
  }

  load_context ctx;
  ctx.arg.mrb = mrb;
  ctx.arg.active = true;
  ctx.arg.src = mrb_nil_p(str) ? port : str;
  ctx.arg.is_io = mrb_nil_p(str);
  ctx.arg.offset = 0;
  ctx.arg.proc = mrb_nil_value();
  ctx.result = mrb_nil_value();

  if (!mrb_nil_p(proc)) {
    ctx.arg.proc = proc;
    gc_roots_push(mrb, &ctx.arg.gc_roots, proc);
  }
  gc_roots_push(mrb, &ctx.arg.gc_roots, ctx.arg.src);

  mrb_bool err = FALSE;
  mrb_value result = mrb_protect_error(mrb, marshal_load_body, &ctx, &err);
  clear_load_arg(&ctx.arg);
  if (err) {
    mrb_exc_raise(mrb, result);
  }
  return result;
}

static mrb_value marshal_load(mrb_state *mrb, mrb_value mod)
{
  mrb_value source, proc = mrb_nil_value();

  mrb_get_args(mrb, "o|o", &source, &proc);
  return mrb_marshal_load_with_proc(mrb, source, proc);
}

// ---------------------------------------------------------------------------
// Module definition
// ---------------------------------------------------------------------------

}  // namespace

void init_marshal(mrb_state *mrb)
{
  s_dump = mrb_intern_lit(mrb, "_dump");
  s_load = mrb_intern_lit(mrb, "_load");
  s_mdump = mrb_intern_lit(mrb, "marshal_dump");
  s_mload = mrb_intern_lit(mrb, "marshal_load");
  s_dump_data = mrb_intern_lit(mrb, "_dump_data");
  s_load_data = mrb_intern_lit(mrb, "_load_data");
  s_call = mrb_intern_lit(mrb, "call");
  s_getbyte = mrb_intern_lit(mrb, "getbyte");
  s_read = mrb_intern_lit(mrb, "read");
  s_write = mrb_intern_lit(mrb, "write");
  s_binmode = mrb_intern_lit(mrb, "binmode");
  s_allocate = mrb_intern_lit(mrb, "allocate");
  s_extend = mrb_intern_lit(mrb, "extend");
  s_prepend = mrb_intern_lit(mrb, "prepend");
  s_to_str = mrb_intern_lit(mrb, "to_str");
  s_excl = mrb_intern_lit(mrb, "excl");
  s_begin = mrb_intern_lit(mrb, "begin");
  s_end = mrb_intern_lit(mrb, "end");
  s_new = mrb_intern_lit(mrb, "new");
  s_source = mrb_intern_lit(mrb, "source");
  s_options = mrb_intern_lit(mrb, "options");

  struct RClass *m = mrb_define_module(mrb, "Marshal");
  mrb_define_class_method(mrb, m, "dump", marshal_dump, MRB_ARGS_ARG(1, 2));
  mrb_define_class_method(mrb, m, "load", marshal_load, MRB_ARGS_ARG(1, 1));
}

extern "C" void mrb_mruby_marshal_gem_init(mrb_state *mrb)
{
  init_marshal(mrb);
}

extern "C" void mrb_mruby_marshal_gem_final(mrb_state *mrb)
{
}
