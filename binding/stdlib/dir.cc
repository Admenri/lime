// CRuby-compatible Dir implementation backed by the engine's virtual file
// system (rgssx::IOService / PhysicsFS). Complements mruby, which ships no
// Dir class by default.
//
// See: https://ruby-doc.org/core-3.0.0/Dir.html

#include "dir.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <string>
#include <vector>

#include "mruby/array.h"
#include "mruby/class.h"
#include "mruby/data.h"
#include "mruby/error.h"
#include "mruby/hash.h"
#include "mruby/string.h"
#include "mruby/variable.h"

#include "mruby_utils.h"

#include "src/filesystem.h"

namespace binding {

// ---------------------------------------------------------------------------
// File::FNM_* flags used by Dir.glob.
// ---------------------------------------------------------------------------
// The engine's virtual filesystem matches names case-insensitively (like the
// existing glob), so FNM_CASEFOLD is effectively always on.
enum {
  FNM_NOESCAPE = 0x01,
  FNM_PATHNAME = 0x02,
  FNM_DOTMATCH = 0x04,
  FNM_CASEFOLD = 0x08,
  FNM_EXTGLOB = 0x10,
};

// IOError is not part of the default mruby core; define it once so Dir
// operations raise the same exception class as CRuby.
static RClass* g_io_error_class = nullptr;

// ---------------------------------------------------------------------------
// Dir data
// ---------------------------------------------------------------------------

struct DirData {
  std::string path;
  std::vector<std::string> entries;
  size_t pos = 0;
  bool closed = false;
};

static void DirDataFree(mrb_state* mrb, void* ptr) {
  delete static_cast<DirData*>(ptr);
}

static const mrb_data_type kDirDataType = {"Dir", DirDataFree};

static DirData* GetDirData(mrb_state* mrb, mrb_value self) {
  return static_cast<DirData*>(mrb_data_get_ptr(mrb, self, &kDirDataType));
}

static void RaiseIOError(mrb_state* mrb, const std::string& msg) {
  mrb_raise(mrb, g_io_error_class, msg.c_str());
}

// The engine's virtual filesystem has no real process working directory, so
// Dir.chdir tracks a virtual current directory ("" = root) used by
// Dir.pwd/getwd and by relative-path resolution.
static std::string g_current_dir = "/";

// ---------------------------------------------------------------------------
// Path helpers
// ---------------------------------------------------------------------------

static std::vector<std::string> ListEntries(const std::string& dir) {
  return rgssx::IOService::Instance()->EnumDir(dir);
}

static bool PathIsDirectory(const std::string& path) {
  // The virtual root is always a directory (PhysicsFS cannot stat ""/".").
  if (path.empty() || path == "/")
    return true;
  return rgssx::IOService::Instance()->IsDirectory(path);
}

// Joins two path components with a single '/' separator.
static std::string JoinPath(const std::string& a, const std::string& b) {
  if (a.empty()) return b;
  if (b.empty()) return a;
  return a + "/" + b;
}

// Splits a '/' separated path into segments.
static std::vector<std::string> SplitPath(const std::string& path) {
  std::vector<std::string> segs;
  size_t start = 0;
  for (size_t i = 0; i <= path.size(); ++i) {
    if (i == path.size() || path[i] == '/') {
      if (i > start)
        segs.push_back(path.substr(start, i - start));
      start = i + 1;
    }
  }
  return segs;
}

// Resolves a user-supplied virtual path for PhysicsFS. PhysicsFS rejects ".",
// ".." and '\' outright, so they are normalized here:
//   - relative paths resolve against the current virtual directory,
//   - absolute paths (leading '/') resolve against the root,
//   - '.' segments are dropped, '..' pops one level,
//   - backslashes are converted to '/'.
// The empty result means the virtual root. Patterns for Dir.glob go through
// the same resolver, keeping their wildcard segments intact.
static std::string NormalizePath(const std::string& input) {
  std::string work = g_current_dir;
  if (!input.empty() && (input[0] == '/' || input[0] == '\\'))
    work.clear();  // absolute path -> root

  std::vector<std::string> base = SplitPath(work);
  std::string p = input;
  for (auto& c : p) {
    if (c == '\\')
      c = '/';
  }

  for (const auto& seg : SplitPath(p)) {
    if (seg == "." || seg.empty())
      continue;
    if (seg == "..") {
      if (!base.empty())
        base.pop_back();
      continue;
    }
    base.push_back(seg);
  }

  std::string result;
  for (const auto& s : base) {
    if (!result.empty())
      result += "/";
    result += s;
  }
  return result;
}

// Resolves the leading ".", ".." and '/' segments of a glob pattern against
// `base` (a root-relative virtual directory). `base` is adjusted in place and
// the remaining pattern (with its wildcards intact) is returned. Everything
// after the first real segment is left untouched so that "a/../*.rb" still
// walks "a" first.
static std::string ResolvePatternPrefix(const std::string& pattern,
                                        std::string& base) {
  std::vector<std::string> base_segs = SplitPath(base);
  size_t i = 0;
  const size_t len = pattern.size();
  while (i < len) {
    if (pattern[i] == '/') {
      ++i;
      continue;
    }
    size_t j = pattern.find_first_of('/', i);
    if (j == std::string::npos)
      j = len;
    std::string seg = pattern.substr(i, j - i);
    if (seg == ".") {
      // stay in place
    } else if (seg == "..") {
      if (!base_segs.empty())
        base_segs.pop_back();
    } else {
      break;  // first real segment
    }
    i = j + 1;
  }

  base.clear();
  for (const auto& s : base_segs) {
    if (!base.empty())
      base += "/";
    base += s;
  }
  return pattern.substr(i);
}

// Builds the full entry list of a directory. PhysicsFS never reports "." or
// "..", so they are inserted by hand to match CRuby's Dir.entries output.
static std::vector<std::string> MakeFullEntries(const std::string& path) {
  std::vector<std::string> entries;
  entries.push_back(".");
  entries.push_back("..");
  auto listed = ListEntries(path);
  entries.insert(entries.end(), listed.begin(), listed.end());
  return entries;
}

// ---------------------------------------------------------------------------
// Glob matching
// ---------------------------------------------------------------------------

// Case-insensitive single-segment glob match supporting '*', '?', '[set]'
// and backslash escapes. Unless dotmatch is set, a wildcard at the start of
// the name does not match a leading '.', matching CRuby without
// FNM_DOTMATCH. name_start marks the first character of the (sub)name so the
// leading-dot rule is only applied at the segment boundary.
static bool GlobSegmentMatch(const char* pat,
                             const char* name,
                             const char* name_start,
                             bool dotmatch) {
  for (;;) {
    char pc = *pat;
    if (pc == '\0')
      return *name == '\0';

    if (pc == '\\') {
      if (pat[1] == '\0')
        return false;  // trailing backslash never matches
      ++pat;
      pc = *pat;
      if (*name != pc)
        return false;
      ++pat;
      ++name;
      continue;
    }

    if (pc == '*') {
      while (*pat == '*')
        ++pat;
      if (!dotmatch && name == name_start && *name == '.')
        return false;  // '*' cannot match a leading dot
      if (*pat == '\0')
        return true;  // trailing '*' matches the rest
      for (const char* n = name;; ++n) {
        if (GlobSegmentMatch(pat, n, name_start, dotmatch))
          return true;
        if (*n == '\0')
          break;
      }
      return false;
    }

    if (pc == '?') {
      if (!dotmatch && name == name_start && *name == '.')
        return false;
      if (*name == '\0')
        return false;
      ++pat;
      ++name;
      continue;
    }

    if (pc == '[') {
      if (!dotmatch && name == name_start && *name == '.')
        return false;
      const char* close = strchr(pat + 1, ']');
      if (!close) {
        // Unmatched '[' is treated as a literal character.
        if (*name != '[')
          return false;
        ++pat;
        ++name;
        continue;
      }
      if (*name == '\0')
        return false;
      char c = *name;
      const char* q = pat + 1;
      bool negate = false;
      if (*q == '^' || *q == '!') {
        negate = true;
        ++q;
      }
      bool matched = false;
      while (q < close) {
        if (q + 2 < close && q[1] == '-') {
          char lo = q[0], hi = q[2];
          if (std::tolower((unsigned char)c) >=
                  std::tolower((unsigned char)lo) &&
              std::tolower((unsigned char)c) <=
                  std::tolower((unsigned char)hi))
            matched = true;
          q += 3;
        } else {
          if (std::tolower((unsigned char)c) ==
              std::tolower((unsigned char)*q))
            matched = true;
          ++q;
        }
      }
      if (matched == negate)
        return false;
      pat = close + 1;
      ++name;
      continue;
    }

    // Literal character.
    if (std::tolower((unsigned char)pc) !=
        std::tolower((unsigned char)*name))
      return false;
    ++pat;
    ++name;
  }
}

// Expands '{a,b,c}' alternations into one concrete pattern per branch.
// Nested braces and escaped braces/commas are supported.
static void ExpandBraces(const std::string& pattern,
                         std::vector<std::string>& out) {
  int depth = 0;
  size_t open = std::string::npos;
  for (size_t i = 0; i < pattern.size(); ++i) {
    char c = pattern[i];
    if (c == '\\') {
      ++i;
      continue;
    }
    if (c == '{') {
      if (depth == 0)
        open = i;
      ++depth;
    } else if (c == '}' && depth > 0) {
      --depth;
      if (depth == 0) {
        // Collect the top-level comma positions inside this pair.
        std::vector<size_t> commas;
        int d = 0;
        for (size_t j = open + 1; j < i; ++j) {
          char cc = pattern[j];
          if (cc == '\\') {
            ++j;
            continue;
          }
          if (cc == '{')
            ++d;
          else if (cc == '}')
            --d;
          else if (cc == ',' && d == 0)
            commas.push_back(j);
        }
        // Without a top-level comma "{x}" is kept literal, like CRuby.
        if (commas.empty()) {
          out.push_back(pattern);
          return;
        }
        std::string prefix = pattern.substr(0, open);
        std::string suffix = pattern.substr(i + 1);
        size_t start = open + 1;
        for (size_t cm : commas) {
          std::vector<std::string> sub;
          ExpandBraces(prefix + pattern.substr(start, cm - start) + suffix,
                       sub);
          out.insert(out.end(), sub.begin(), sub.end());
          start = cm + 1;
        }
        std::vector<std::string> last;
        ExpandBraces(prefix + pattern.substr(start, i - start) + suffix, last);
        out.insert(out.end(), last.begin(), last.end());
        return;
      }
    }
  }
  out.push_back(pattern);
}

// Recursively expands a single (brace-expanded) glob pattern against the
// virtual file tree.
//   base:   directory the pattern is relative to ("" = virtual root).
//   prefix: already matched directory path relative to base ("" = base).
//   segs:   pattern split on '/' (or '\').
//   idx:    current segment index.
//   results: matched paths, relative to base.
static void GlobWalk(const std::string& base,
                     const std::string& prefix,
                     const std::vector<std::string>& segs,
                     size_t idx,
                     bool dotmatch,
                     std::vector<std::string>& results) {
  if (idx >= segs.size())
    return;

  const std::string& seg = segs[idx];
  bool is_last = (idx + 1 == segs.size());
  std::string enum_dir = JoinPath(base, prefix);

  if (seg == "**") {
    if (is_last) {
      // A trailing '**' matches every entry recursively (files and dirs).
      for (const auto& name : ListEntries(enum_dir)) {
        if (!dotmatch && !name.empty() && name[0] == '.')
          continue;
        std::string full = JoinPath(prefix, name);
        results.push_back(full);
        if (PathIsDirectory(JoinPath(base, full)))
          GlobWalk(base, full, segs, idx, dotmatch, results);
      }
      return;
    }
    // '**/' matches zero or more directory levels.
    GlobWalk(base, prefix, segs, idx + 1, dotmatch, results);
    for (const auto& name : ListEntries(enum_dir)) {
      if (!dotmatch && !name.empty() && name[0] == '.')
        continue;
      std::string full = JoinPath(prefix, name);
      if (PathIsDirectory(JoinPath(base, full)))
        GlobWalk(base, full, segs, idx, dotmatch, results);
    }
    return;
  }

  for (const auto& name : ListEntries(enum_dir)) {
    if (!GlobSegmentMatch(seg.c_str(), name.c_str(), name.c_str(), dotmatch))
      continue;
    std::string full = JoinPath(prefix, name);
    if (is_last) {
      results.push_back(full);
    } else if (PathIsDirectory(JoinPath(base, full))) {
      GlobWalk(base, full, segs, idx + 1, dotmatch, results);
    }
  }
}

struct GlobOptions {
  bool dotmatch = false;
  std::string base;
  bool sort = true;
};

// Parses the arguments of Dir.glob / Dir.[]. A trailing Hash is treated as
// keyword arguments (base:, sort:, flags:); positional Integers are glob
// flags; everything else must be a String or an Array of Strings and is
// collected as a pattern.
static void ParseGlobArgs(mrb_state* mrb,
                          const mrb_value* argv,
                          mrb_int argc,
                          std::vector<std::string>& patterns,
                          GlobOptions& opts) {
  const mrb_value* end = argv + argc;
  if (argc > 0 && mrb_hash_p(argv[argc - 1])) {
    --end;
    mrb_value kw = *end;
    mrb_sym base_sym = mrb_intern_lit(mrb, "base");
    mrb_sym sort_sym = mrb_intern_lit(mrb, "sort");
    mrb_sym flags_sym = mrb_intern_lit(mrb, "flags");
    if (mrb_hash_key_p(mrb, kw, mrb_symbol_value(base_sym))) {
      mrb_value base_val = mrb_hash_get(mrb, kw, mrb_symbol_value(base_sym));
      if (!mrb_nil_p(base_val))
        opts.base = MRBStringValue(base_val);
    }
    if (mrb_hash_key_p(mrb, kw, mrb_symbol_value(sort_sym)))
      opts.sort = mrb_test(mrb_hash_get(mrb, kw, mrb_symbol_value(sort_sym)));
    if (mrb_hash_key_p(mrb, kw, mrb_symbol_value(flags_sym)))
      opts.dotmatch =
          (mrb_as_int(mrb, mrb_hash_get(mrb, kw, mrb_symbol_value(flags_sym))) &
           FNM_DOTMATCH) != 0;
  }

  for (const mrb_value* a = argv; a != end; ++a) {
    if (mrb_string_p(*a)) {
      patterns.push_back(MRBStringValue(*a));
    } else if (mrb_array_p(*a)) {
      mrb_int len = RARRAY_LEN(*a);
      const mrb_value* ptr = RARRAY_PTR(*a);
      for (mrb_int i = 0; i < len; ++i) {
        if (!mrb_string_p(ptr[i]))
          mrb_raise(mrb, E_TYPE_ERROR, "array element is not a String");
        patterns.push_back(MRBStringValue(ptr[i]));
      }
    } else if (mrb_integer_p(*a)) {
      opts.dotmatch = (mrb_as_int(mrb, *a) & FNM_DOTMATCH) != 0;
    } else if (mrb_nil_p(*a)) {
      // Ignore a nil flags placeholder.
    } else {
      mrb_raise(mrb, E_TYPE_ERROR,
                "pattern must be a String or an Array of Strings");
    }
  }
}

// Core glob implementation shared by Dir.glob and Dir.[]. With a block each
// match is yielded and nil is returned; otherwise an Array is returned.
static mrb_value GlobPatterns(mrb_state* mrb,
                              const std::vector<std::string>& patterns,
                              const GlobOptions& opts,
                              mrb_value block) {
  std::vector<std::string> matches;
  // Root-relative enumeration base: the current virtual directory, or the
  // explicit base: option. Glob results are returned relative to this base
  // (matching CRuby's behavior after Dir.chdir).
  std::string base = NormalizePath(opts.base);

  for (const auto& pattern : patterns) {
    std::string p = pattern;
    for (auto& c : p) {
      if (c == '\\')
        c = '/';
    }

    // Absolute patterns are always relative to the virtual root.
    std::string walk_base = (!p.empty() && p[0] == '/') ? "" : base;
    std::string rest = ResolvePatternPrefix(p, walk_base);
    if (rest.empty())
      continue;  // pattern collapsed to the base directory (e.g. "" or ".")

    std::vector<std::string> expanded;
    ExpandBraces(rest, expanded);

    for (const auto& e : expanded) {
      std::vector<std::string> segs = SplitPath(e);
      if (segs.empty())
        continue;
      GlobWalk(walk_base, "", segs, 0, opts.dotmatch, matches);
    }
  }

  if (opts.sort)
    std::sort(matches.begin(), matches.end());

  mrb_value result = mrb_ary_new(mrb);
  auto ai = mrb_gc_arena_save(mrb);
  for (const auto& m : matches) {
    mrb_value s = mrb_str_new_cstr(mrb, m.c_str());
    if (mrb_nil_p(block))
      mrb_ary_push(mrb, result, s);
    else
      mrb_yield(mrb, block, s);
    mrb_gc_arena_restore(mrb, ai);
  }
  mrb_gc_arena_restore(mrb, ai);

  // With a block each match is yielded and nil is returned; otherwise the
  // Array of matches is returned.
  return mrb_nil_p(block) ? result : mrb_nil_value();
}

// ---------------------------------------------------------------------------
// Dir class methods
// ---------------------------------------------------------------------------

MRB_FUNC(dir_glob) {
  const mrb_value* argv;
  mrb_int argc;
  mrb_value block;
  mrb_get_args(mrb, "*&", &argv, &argc, &block);

  std::vector<std::string> patterns;
  GlobOptions opts;
  ParseGlobArgs(mrb, argv, argc, patterns, opts);

  return GlobPatterns(mrb, patterns, opts, block);
}

MRB_FUNC(dir_brackets) {
  const mrb_value* argv;
  mrb_int argc;
  mrb_get_args(mrb, "*", &argv, &argc);

  std::vector<std::string> patterns;
  GlobOptions opts;
  ParseGlobArgs(mrb, argv, argc, patterns, opts);

  return GlobPatterns(mrb, patterns, opts, mrb_nil_value());
}

MRB_FUNC(dir_mkdir) {
  const char* path;
  mrb_int mode = 0777;
  mrb_get_args(mrb, "z|i", &path, &mode);
  if (!rgssx::IOService::Instance()->Mkdir(NormalizePath(path))) {
    std::string msg = "mkdir: " + rgssx::IOService::Instance()->GetLastError();
    RaiseIOError(mrb, msg);
  }
  return mrb_fixnum_value(0);
}

MRB_FUNC(dir_delete) {
  const char* path;
  mrb_get_args(mrb, "z", &path);
  if (!rgssx::IOService::Instance()->Rmdir(NormalizePath(path))) {
    std::string msg =
        "rmdir: " + rgssx::IOService::Instance()->GetLastError();
    RaiseIOError(mrb, msg);
  }
  return mrb_fixnum_value(0);
}

MRB_FUNC(dir_existp) {
  const char* path;
  mrb_get_args(mrb, "z", &path);
  return mrb_bool_value(PathIsDirectory(NormalizePath(path)));
}

MRB_FUNC(dir_empty_p) {
  const char* path;
  mrb_get_args(mrb, "z", &path);
  std::string norm = NormalizePath(path);
  if (!PathIsDirectory(norm))
    return mrb_false_value();
  return mrb_bool_value(ListEntries(norm).empty());
}

static void EnsureDirectory(mrb_state* mrb, const std::string& path) {
  if (!PathIsDirectory(NormalizePath(path)))
    RaiseIOError(mrb, "not a directory: " + path);
}

MRB_FUNC(dir_entries) {
  const char* path;
  mrb_get_args(mrb, "z", &path);
  EnsureDirectory(mrb, path);
  return WrapStringVector(mrb, MakeFullEntries(NormalizePath(path)));
}

MRB_FUNC(dir_children) {
  const char* path;
  mrb_get_args(mrb, "z", &path);
  EnsureDirectory(mrb, path);
  return WrapStringVector(mrb, ListEntries(NormalizePath(path)));
}

// Yields every entry of an open Dir object to the block, optionally skipping
// "." and "..".
static void YieldDirEntries(mrb_state* mrb,
                            mrb_value dir_obj,
                            bool skip_special,
                            mrb_value block) {
  auto* data = GetDirData(mrb, dir_obj);
  auto ai = mrb_gc_arena_save(mrb);
  for (const auto& e : data->entries) {
    if (skip_special && (e == "." || e == ".."))
      continue;
    mrb_value s = mrb_str_new_cstr(mrb, e.c_str());
    mrb_yield(mrb, block, s);
    mrb_gc_arena_restore(mrb, ai);
  }
  mrb_gc_arena_restore(mrb, ai);
}

MRB_FUNC(dir_foreach) {
  const char* path;
  mrb_value block;
  mrb_get_args(mrb, "z&", &path, &block);

  RClass* dir_class = mrb_class_get(mrb, "Dir");
  mrb_value path_str = mrb_str_new_cstr(mrb, path);
  mrb_value dir =
      mrb_funcall(mrb, mrb_obj_value(dir_class), "new", 1, path_str);

  if (mrb_nil_p(block))
    return mrb_funcall(mrb, mrb_obj_value(dir_class), "to_enum", 2,
                       mrb_symbol_value(mrb_intern_lit(mrb, "foreach")),
                       path_str);

  YieldDirEntries(mrb, dir, false, block);
  return mrb_nil_value();
}

MRB_FUNC(dir_each_child) {
  const char* path;
  mrb_value block;
  mrb_get_args(mrb, "z&", &path, &block);

  RClass* dir_class = mrb_class_get(mrb, "Dir");
  mrb_value path_str = mrb_str_new_cstr(mrb, path);
  mrb_value dir =
      mrb_funcall(mrb, mrb_obj_value(dir_class), "new", 1, path_str);

  if (mrb_nil_p(block))
    return mrb_funcall(mrb, mrb_obj_value(dir_class), "to_enum", 2,
                       mrb_symbol_value(mrb_intern_lit(mrb, "each_child")),
                       path_str);

  YieldDirEntries(mrb, dir, true, block);
  return mrb_nil_value();
}

MRB_FUNC(dir_getwd) {
  return mrb_str_new_cstr(mrb,
                          g_current_dir.empty() ? "/" : g_current_dir.c_str());
}

MRB_FUNC(dir_chdir) {
  const char* path = nullptr;
  mrb_value block;
  mrb_get_args(mrb, "|z&", &path, &block);

  std::string target = NormalizePath(path ? path : "/");
  if (!PathIsDirectory(target))
    RaiseIOError(mrb, std::string("no such directory: ") + (path ? path : "/"));

  std::string old = g_current_dir;
  g_current_dir = target;

  if (mrb_nil_p(block))
    return mrb_fixnum_value(0);

  mrb_value result =
      mrb_yield(mrb, block, mrb_str_new_cstr(mrb, target.c_str()));
  g_current_dir = old;
  return result;
}

MRB_FUNC(dir_open) {
  const char* path;
  mrb_value block;
  mrb_get_args(mrb, "z&", &path, &block);

  RClass* dir_class = mrb_class_get(mrb, "Dir");
  mrb_value dir =
      mrb_funcall(mrb, mrb_obj_value(dir_class), "new", 1,
                  mrb_str_new_cstr(mrb, path));
  if (mrb_nil_p(block))
    return dir;

  mrb_value result = mrb_yield(mrb, block, dir);
  mrb_funcall(mrb, dir, "close", 0);
  return result;
}

MRB_FUNC(dir_chroot) {
  mrb_raise(mrb, E_NOTIMP_ERROR, "chroot() not supported on this platform");
  return mrb_nil_value();
}

MRB_FUNC(dir_home) {
  mrb_raise(mrb, E_NOTIMP_ERROR, "Dir.home not supported on this platform");
  return mrb_nil_value();
}

// ---------------------------------------------------------------------------
// Dir instance methods
// ---------------------------------------------------------------------------

MRB_FUNC(dir_initialize) {
  const char* path;
  mrb_get_args(mrb, "z", &path);

  EnsureDirectory(mrb, path);

  // Release any previous payload (e.g. re-initialization via #send).
  if (DATA_PTR(self))
    DirDataFree(mrb, DATA_PTR(self));

  auto* data = new DirData();
  // #path / #to_path / #inspect keep the caller's spelling (like CRuby).
  data->path = path;
  data->entries = MakeFullEntries(NormalizePath(path));

  DATA_PTR(self) = data;
  DATA_TYPE(self) = &kDirDataType;
  return self;
}

MRB_FUNC(dir_close) {
  auto* data = GetDirData(mrb, self);
  data->closed = true;
  return mrb_nil_value();
}

MRB_FUNC(dir_read) {
  auto* data = GetDirData(mrb, self);
  if (data->closed)
    RaiseIOError(mrb, "closed directory");
  if (data->pos >= data->entries.size())
    return mrb_nil_value();
  return mrb_str_new_cstr(mrb, data->entries[data->pos++].c_str());
}

MRB_FUNC(dir_rewind) {
  auto* data = GetDirData(mrb, self);
  if (data->closed)
    RaiseIOError(mrb, "closed directory");
  data->pos = 0;
  return self;
}

MRB_FUNC(dir_seek) {
  auto* data = GetDirData(mrb, self);
  if (data->closed)
    RaiseIOError(mrb, "closed directory");
  mrb_int pos;
  mrb_get_args(mrb, "i", &pos);
  if (pos < 0)
    mrb_raise(mrb, E_ARGUMENT_ERROR, "negative directory position");
  data->pos = static_cast<size_t>(pos);
  return self;
}

MRB_FUNC(dir_pos_set) {
  auto* data = GetDirData(mrb, self);
  if (data->closed)
    RaiseIOError(mrb, "closed directory");
  mrb_int pos;
  mrb_get_args(mrb, "i", &pos);
  if (pos < 0)
    mrb_raise(mrb, E_ARGUMENT_ERROR, "negative directory position");
  data->pos = static_cast<size_t>(pos);
  return mrb_fixnum_value(pos);
}

MRB_FUNC(dir_tell) {
  auto* data = GetDirData(mrb, self);
  if (data->closed)
    RaiseIOError(mrb, "closed directory");
  return mrb_fixnum_value(static_cast<mrb_int>(data->pos));
}

MRB_FUNC(dir_path) {
  auto* data = GetDirData(mrb, self);
  return mrb_str_new_cstr(mrb, data->path.c_str());
}

MRB_FUNC(dir_inspect) {
  auto* data = GetDirData(mrb, self);
  mrb_value str = mrb_str_buf_new(mrb, data->path.size() + 16);
  mrb_str_cat_lit(mrb, str, "#<Dir:");
  mrb_str_cat_str(mrb, str, mrb_str_new_cstr(mrb, data->path.c_str()));
  mrb_str_cat_lit(mrb, str, ">");
  return str;
}

MRB_FUNC(dir_children_inst) {
  auto* data = GetDirData(mrb, self);
  if (data->closed)
    RaiseIOError(mrb, "closed directory");
  mrb_value ary = mrb_ary_new(mrb);
  auto ai = mrb_gc_arena_save(mrb);
  for (const auto& e : data->entries) {
    if (e == "." || e == "..")
      continue;
    mrb_ary_push(mrb, ary, mrb_str_new_cstr(mrb, e.c_str()));
    mrb_gc_arena_restore(mrb, ai);
  }
  mrb_gc_arena_restore(mrb, ai);
  return ary;
}

MRB_FUNC(dir_each) {
  auto* data = GetDirData(mrb, self);
  if (data->closed)
    RaiseIOError(mrb, "closed directory");
  mrb_value block;
  mrb_get_args(mrb, "&", &block);
  if (mrb_nil_p(block))
    return mrb_funcall(mrb, self, "to_enum", 1,
                       mrb_symbol_value(mrb_intern_lit(mrb, "each")));

  auto ai = mrb_gc_arena_save(mrb);
  while (data->pos < data->entries.size()) {
    mrb_value s = mrb_str_new_cstr(mrb, data->entries[data->pos++].c_str());
    mrb_yield(mrb, block, s);
    mrb_gc_arena_restore(mrb, ai);
  }
  mrb_gc_arena_restore(mrb, ai);
  return self;
}

MRB_FUNC(dir_each_child_inst) {
  auto* data = GetDirData(mrb, self);
  if (data->closed)
    RaiseIOError(mrb, "closed directory");
  mrb_value block;
  mrb_get_args(mrb, "&", &block);
  if (mrb_nil_p(block))
    return mrb_funcall(mrb, self, "to_enum", 1,
                       mrb_symbol_value(mrb_intern_lit(mrb, "each_child")));

  auto ai = mrb_gc_arena_save(mrb);
  while (data->pos < data->entries.size()) {
    const std::string& e = data->entries[data->pos++];
    if (e == "." || e == "..")
      continue;
    mrb_value s = mrb_str_new_cstr(mrb, e.c_str());
    mrb_yield(mrb, block, s);
    mrb_gc_arena_restore(mrb, ai);
  }
  mrb_gc_arena_restore(mrb, ai);
  return self;
}

MRB_FUNC(dir_fileno) {
  mrb_raise(mrb, E_NOTIMP_ERROR, "Dir#fileno not supported on this platform");
  return mrb_nil_value();
}

// ---------------------------------------------------------------------------
// Initialization
// ---------------------------------------------------------------------------

void InitStdlibDir(mrb_state* mrb) {
  // IOError is referenced by Dir but absent from the default mruby core.
  g_io_error_class = mrb_define_class(mrb, "IOError", mrb->eStandardError_class);

  RClass* dir_class = mrb_define_class(mrb, "Dir", mrb->object_class);
  MRB_SET_INSTANCE_TT(dir_class, MRB_TT_CDATA);
  mrb_include_module(mrb, dir_class, mrb_module_get(mrb, "Enumerable"));

  // Class methods
  mrb_define_class_method(mrb, dir_class, "[]", dir_brackets,
                          MRB_ARGS_REST());
  mrb_define_class_method(mrb, dir_class, "chdir", dir_chdir,
                          MRB_ARGS_ARG(0, 1) | MRB_ARGS_BLOCK());
  mrb_define_class_method(mrb, dir_class, "chroot", dir_chroot,
                          MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, dir_class, "children", dir_children,
                          MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, dir_class, "delete", dir_delete,
                          MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, dir_class, "each_child", dir_each_child,
                          MRB_ARGS_REQ(1) | MRB_ARGS_BLOCK());
  mrb_define_class_method(mrb, dir_class, "empty?", dir_empty_p,
                          MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, dir_class, "entries", dir_entries,
                          MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, dir_class, "exist?", dir_existp,
                          MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, dir_class, "exists?", dir_existp,
                          MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, dir_class, "foreach", dir_foreach,
                          MRB_ARGS_REQ(1) | MRB_ARGS_BLOCK());
  mrb_define_class_method(mrb, dir_class, "getwd", dir_getwd, MRB_ARGS_NONE());
  mrb_define_class_method(mrb, dir_class, "glob", dir_glob,
                          MRB_ARGS_REST() | MRB_ARGS_BLOCK());
  mrb_define_class_method(mrb, dir_class, "home", dir_home, MRB_ARGS_OPT(1));
  mrb_define_class_method(mrb, dir_class, "mkdir", dir_mkdir,
                          MRB_ARGS_ARG(1, 1));
  mrb_define_class_method(mrb, dir_class, "open", dir_open,
                          MRB_ARGS_REQ(1) | MRB_ARGS_BLOCK());
  mrb_define_class_method(mrb, dir_class, "pwd", dir_getwd, MRB_ARGS_NONE());
  mrb_define_class_method(mrb, dir_class, "rmdir", dir_delete,
                          MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, dir_class, "unlink", dir_delete,
                          MRB_ARGS_REQ(1));

  // Instance methods
  mrb_define_method(mrb, dir_class, "initialize", dir_initialize,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, dir_class, "children", dir_children_inst,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, dir_class, "close", dir_close, MRB_ARGS_NONE());
  mrb_define_method(mrb, dir_class, "each", dir_each, MRB_ARGS_BLOCK());
  mrb_define_method(mrb, dir_class, "each_child", dir_each_child_inst,
                    MRB_ARGS_BLOCK());
  mrb_define_method(mrb, dir_class, "fileno", dir_fileno, MRB_ARGS_NONE());
  mrb_define_method(mrb, dir_class, "inspect", dir_inspect, MRB_ARGS_NONE());
  mrb_define_method(mrb, dir_class, "path", dir_path, MRB_ARGS_NONE());
  mrb_define_method(mrb, dir_class, "pos", dir_tell, MRB_ARGS_NONE());
  mrb_define_method(mrb, dir_class, "pos=", dir_pos_set, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, dir_class, "read", dir_read, MRB_ARGS_NONE());
  mrb_define_method(mrb, dir_class, "rewind", dir_rewind, MRB_ARGS_NONE());
  mrb_define_method(mrb, dir_class, "seek", dir_seek, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, dir_class, "tell", dir_tell, MRB_ARGS_NONE());
  mrb_define_method(mrb, dir_class, "to_path", dir_path, MRB_ARGS_NONE());
}

}  // namespace binding
