cmake_minimum_required(VERSION 3.30)

set(PRISM_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/mruby/mrbgems/mruby-compiler/lib/prism")
add_subdirectory(prism_parser)

set(ONIG_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/oniguruma")
add_subdirectory(oniguruma)

set(PRESYM_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/include")
set(PRESYM_HEADER_DIR "${PRESYM_INCLUDE_DIR}/mruby/presym")

# ═══════════════════════════════════════════════════════════════════════════════
# mruby library target
# ═══════════════════════════════════════════════════════════════════════════════
add_library(mruby STATIC)
file(GLOB MRUBY_CORE_SOURCES "mruby/src/*.c")
target_sources(mruby PUBLIC ${MRUBY_CORE_SOURCES})
target_include_directories(mruby PUBLIC "mruby/include" ${PRESYM_INCLUDE_DIR})
target_link_libraries(mruby PUBLIC prism onig)
if(WIN32)
  # mruby-io uses Winsock APIs (closesocket/getsockopt/WSAGetLastError)
  target_link_libraries(mruby PUBLIC ws2_32)
endif()
target_compile_definitions(mruby
 PUBLIC
  MRC_TARGET_MRUBY=1
  MRB_USE_BIGINT=1
  MRB_USE_RATIONAL=1
  MRB_UTF8_STRING=1
  MRB_NO_BOXING=1
  MRB_SYMBOL_MAX=0
  MRB_STR_LENGTH_MAX=0
  MRB_ARY_LENGTH_MAX=0
  MRB_FUNCALL_ARGC_MAX=32
)

# ═══════════════════════════════════════════════════════════════════════════════
# mruby gems
# ═══════════════════════════════════════════════════════════════════════════════
set(MRUBY_GEMS
  # stdlib
  mruby-compar-ext
  mruby-enum-ext
  mruby-string-ext
  mruby-numeric-ext
  mruby-array-ext
  mruby-hash-ext
  mruby-range-ext
  mruby-proc-ext
  mruby-symbol-ext
  mruby-object-ext
  mruby-objectspace
  mruby-set
  mruby-fiber
  mruby-enumerator
  mruby-enum-lazy
  mruby-enum-chain
  mruby-toplevel-ext
  mruby-kernel-ext
  mruby-class-ext
  mruby-catch
  # stdlib-ext
  mruby-pack
  mruby-sprintf
  mruby-time
  mruby-struct
  mruby-data
  mruby-random
  # metaprog
  mruby-metaprog
  mruby-method
  mruby-binding
  mruby-eval
  mruby-proc-binding
  mruby-compiler
  # math
  mruby-math
  mruby-rational
  mruby-complex
  mruby-bigint
  # platform
  mruby-io
)

set(EXTERNAL_MRUBY_GEMS
  mruby-marshal
  mruby-onig-regexp
)
set(EXTERNAL_MRUBY_GEMS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/gems")

# ── Process gems: collect sources, generate mrblib C files ───────────────────
set(MRUBY_GEMS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/mruby/mrbgems")
set(MRUBY_GEMS_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/gems")
file(MAKE_DIRECTORY ${MRUBY_GEMS_BUILD_DIR})

set(MRUBY_GEMS_SOURCES "")
set(MRUBY_GEM_C_SOURCES "")     # real .c files from src/ and core/ (for presym)
set(MRUBY_GEM_GEN_SOURCES "")   # generated .c files (mrblib, gems_init)
set(MRUBY_GEMS_ACTIVE "")       # gem names with at least C source or mrblib

function(_gem_func_name gem_name out_var)
  string(REPLACE "-" "_" _fn "${gem_name}")
  set(${out_var} "${_fn}" PARENT_SCOPE)
endfunction()

# Helper: read a file and output as a C byte array literal.
# Output format: "0xNN, 0xNN, ..." (16 bytes per line)
function(_write_c_byte_array file_path out_var)
  file(SIZE "${file_path}" _total)
  set(_result "")
  if(_total EQUAL 0)
    set(${out_var} "" PARENT_SCOPE)
    return()
  endif()
  file(READ "${file_path}" _bytes HEX)
  # _bytes is a hex string like "23202D2D0A..."
  # Each source byte is encoded as 2 hex characters, so the hex string
  # length is twice the file size in bytes.
  math(EXPR _hex_len "${_total} * 2")
  set(_pos 0)
  set(_col 0)
  while(_pos LESS _hex_len)
    string(SUBSTRING "${_bytes}" ${_pos} 2 _hex)
    string(APPEND _result "0x${_hex}")
    math(EXPR _pos "${_pos} + 2")
    if(_pos LESS _hex_len)
      string(APPEND _result ", ")
    endif()
    math(EXPR _col "${_col} + 1")
    if(_col EQUAL 16 AND _pos LESS _hex_len)
      string(APPEND _result "\n")
      set(_col 0)
    endif()
  endwhile()
  set(${out_var} "${_result}" PARENT_SCOPE)
endfunction()

foreach(gem ${MRUBY_GEMS})
  set(gem_dir "${MRUBY_GEMS_DIR}/${gem}")
  set(gem_has_src FALSE)

  # ── Collect C source files from src/ and core/ ────────────────────────────
  if(EXISTS "${gem_dir}/src")
    file(GLOB gem_src_files "${gem_dir}/src/*.c")
    list(LENGTH gem_src_files _n)
    if(_n GREATER 0)
      set(gem_has_src TRUE)
      list(APPEND MRUBY_GEMS_SOURCES ${gem_src_files})
      list(APPEND MRUBY_GEM_C_SOURCES ${gem_src_files})
    endif()
  endif()
  if(EXISTS "${gem_dir}/core")
    file(GLOB gem_core_files "${gem_dir}/core/*.c")
    list(LENGTH gem_core_files _n)
    if(_n GREATER 0)
      set(gem_has_src TRUE)
      list(APPEND MRUBY_GEMS_SOURCES ${gem_core_files})
      list(APPEND MRUBY_GEM_C_SOURCES ${gem_core_files})
    endif()
  endif()

  # ── Collect C source files from ports/ ────────────────────────────────────
  if(WIN32)
    # Windows port
    file(GLOB gem_src_files "${gem_dir}/ports/win/*.c")
    list(LENGTH gem_src_files _n)
    if(_n GREATER 0)
      set(gem_has_src TRUE)
      list(APPEND MRUBY_GEMS_SOURCES ${gem_src_files})
      list(APPEND MRUBY_GEM_C_SOURCES ${gem_src_files})
    endif()
  else()
    # Posix port
    file(GLOB gem_src_files "${gem_dir}/ports/posix/*.c")
    list(LENGTH gem_src_files _n)
    if(_n GREATER 0)
      set(gem_has_src TRUE)
      list(APPEND MRUBY_GEMS_SOURCES ${gem_src_files})
      list(APPEND MRUBY_GEM_C_SOURCES ${gem_src_files})
    endif()
  endif()

  # ── Add gem include dirs for compilation ──────────────────────────────────
  if(EXISTS "${gem_dir}/include")
    target_include_directories(mruby PRIVATE "${gem_dir}/include")
  endif()
  if(EXISTS "${gem_dir}/src")
    target_include_directories(mruby PRIVATE "${gem_dir}/src")
  endif()

  # ── Generate mrblib C file at CONFIGURE time ──────────────────────────────
  set(mrblib_dir "${gem_dir}/mrblib")
  set(gem_mrblib_c "${MRUBY_GEMS_BUILD_DIR}/${gem}_mrblib.c")
  if(EXISTS "${mrblib_dir}")
    file(GLOB rb_files RELATIVE "${mrblib_dir}" "${mrblib_dir}/*.rb")
    list(SORT rb_files)
    if(rb_files)
      _gem_func_name("${gem}" gem_fn)
      set(_mrblib_body "")
      string(APPEND _mrblib_body "/* Auto-generated - DO NOT EDIT */\n")
      string(APPEND _mrblib_body "#include <mruby/compile.h>\n\n")

      foreach(rb ${rb_files})
        set(_rb_path "${mrblib_dir}/${rb}")
        _write_c_byte_array("${_rb_path}" _c_bytes)
        string(REGEX REPLACE "[^a-zA-Z0-9_]" "_" _var "mrblib_${gem_fn}_${rb}")
        string(APPEND _mrblib_body "static const unsigned char ${_var}[] = {\n${_c_bytes}\n};\n\n")
      endforeach()

      string(APPEND _mrblib_body "void mrb_load_gem_${gem_fn}(mrb_state *mrb)\n{\n")
      foreach(rb ${rb_files})
        string(REGEX REPLACE "[^a-zA-Z0-9_]" "_" _var "mrblib_${gem_fn}_${rb}")
        string(APPEND _mrblib_body "  mrb_load_nstring(mrb, (const char*)${_var}, sizeof(${_var}));\n")
      endforeach()
      string(APPEND _mrblib_body "}\n")

      file(WRITE "${gem_mrblib_c}" "${_mrblib_body}")
      set(gem_has_mrblib TRUE)
      message(STATUS "Gem mrblib: ${gem} (${rb_files})")
    endif()
  endif()

  # ── Stub if no mrblib ────────────────────────────────────────────────────
  if(NOT EXISTS "${gem_mrblib_c}")
    _gem_func_name("${gem}" gem_fn)
    file(WRITE "${gem_mrblib_c}"
      "/* Auto-generated empty stub */\n"
      "#include <mruby.h>\n"
      "void mrb_load_gem_${gem_fn}(mrb_state *mrb) { (void)mrb; }\n"
    )
  endif()

  # ── Generate init/final stubs for gems without C source ───────────────────
  if(NOT gem_has_src)
    _gem_func_name("${gem}" gem_fn)
    set(_gem_init_c "${MRUBY_GEMS_BUILD_DIR}/${gem}_init.c")
    file(WRITE "${_gem_init_c}"
      "/* Auto-generated stubs for gem without C source */\n"
      "#include <mruby.h>\n"
      "void mrb_${gem_fn}_gem_init(mrb_state *mrb) { (void)mrb; }\n"
      "void mrb_${gem_fn}_gem_final(mrb_state *mrb) { (void)mrb; }\n"
    )
    list(APPEND MRUBY_GEMS_SOURCES "${_gem_init_c}")
    list(APPEND MRUBY_GEM_C_SOURCES "${_gem_init_c}")
    set(gem_has_src TRUE)  # now it has C source (stubs)
    message(STATUS "Gem init stubs: ${gem}")
  endif()

  list(APPEND MRUBY_GEMS_SOURCES "${gem_mrblib_c}")
  list(APPEND MRUBY_GEM_GEN_SOURCES "${gem_mrblib_c}")

  if(gem_has_src)
    list(APPEND MRUBY_GEMS_ACTIVE "${gem}")
  endif()
endforeach()

# ── Process external gems ──────────────────────────────────────────────────
foreach(gem ${EXTERNAL_MRUBY_GEMS})
  set(gem_dir "${EXTERNAL_MRUBY_GEMS_DIR}/${gem}")
  set(gem_has_src FALSE)

  # ── Collect C source files from src/ and core/ ──────────────────────────
  if(EXISTS "${gem_dir}/src")
    file(GLOB gem_src_files "${gem_dir}/src/*.c")
    list(LENGTH gem_src_files _n)
    if(_n GREATER 0)
      set(gem_has_src TRUE)
      list(APPEND MRUBY_GEMS_SOURCES ${gem_src_files})
      list(APPEND MRUBY_GEM_C_SOURCES ${gem_src_files})
    endif()
  endif()
  if(EXISTS "${gem_dir}/core")
    file(GLOB gem_core_files "${gem_dir}/core/*.c")
    list(LENGTH gem_core_files _n)
    if(_n GREATER 0)
      set(gem_has_src TRUE)
      list(APPEND MRUBY_GEMS_SOURCES ${gem_core_files})
      list(APPEND MRUBY_GEM_C_SOURCES ${gem_core_files})
    endif()
  endif()

  # ── Collect C source files from ports/ ──────────────────────────
  if(WIN32)
    # Windows port
    file(GLOB gem_src_files "${gem_dir}/ports/win/*.c")
    list(LENGTH gem_src_files _n)
    if(_n GREATER 0)
      set(gem_has_src TRUE)
      list(APPEND MRUBY_GEMS_SOURCES ${gem_src_files})
      list(APPEND MRUBY_GEM_C_SOURCES ${gem_src_files})
    endif()
  else()
    # Posix port
    file(GLOB gem_src_files "${gem_dir}/ports/posix/*.c")
    list(LENGTH gem_src_files _n)
    if(_n GREATER 0)
      set(gem_has_src TRUE)
      list(APPEND MRUBY_GEMS_SOURCES ${gem_src_files})
      list(APPEND MRUBY_GEM_C_SOURCES ${gem_src_files})
    endif()
  endif()

  # ── Add gem include dirs for compilation ────────────────────────────────
  if(EXISTS "${gem_dir}/include")
    target_include_directories(mruby PRIVATE "${gem_dir}/include")
  endif()
  if(EXISTS "${gem_dir}/src")
    target_include_directories(mruby PRIVATE "${gem_dir}/src")
  endif()

  # ── Generate mrblib C file at CONFIGURE time ────────────────────────────
  set(mrblib_dir "${gem_dir}/mrblib")
  set(gem_mrblib_c "${MRUBY_GEMS_BUILD_DIR}/${gem}_mrblib.c")
  if(EXISTS "${mrblib_dir}")
    file(GLOB rb_files RELATIVE "${mrblib_dir}" "${mrblib_dir}/*.rb")
    list(SORT rb_files)
    if(rb_files)
      _gem_func_name("${gem}" gem_fn)
      set(_mrblib_body "")
      string(APPEND _mrblib_body "/* Auto-generated - DO NOT EDIT */\n")
      string(APPEND _mrblib_body "#include <mruby/compile.h>\n\n")

      foreach(rb ${rb_files})
        set(_rb_path "${mrblib_dir}/${rb}")
        _write_c_byte_array("${_rb_path}" _c_bytes)
        string(REGEX REPLACE "[^a-zA-Z0-9_]" "_" _var "mrblib_${gem_fn}_${rb}")
        string(APPEND _mrblib_body "static const unsigned char ${_var}[] = {\n${_c_bytes}\n};\n\n")
      endforeach()

      string(APPEND _mrblib_body "void mrb_load_gem_${gem_fn}(mrb_state *mrb)\n{\n")
      foreach(rb ${rb_files})
        string(REGEX REPLACE "[^a-zA-Z0-9_]" "_" _var "mrblib_${gem_fn}_${rb}")
        string(APPEND _mrblib_body "  mrb_load_nstring(mrb, (const char*)${_var}, sizeof(${_var}));\n")
      endforeach()
      string(APPEND _mrblib_body "}\n")

      file(WRITE "${gem_mrblib_c}" "${_mrblib_body}")
      set(gem_has_mrblib TRUE)
      message(STATUS "Gem mrblib: ${gem} (${rb_files})")
    endif()
  endif()

  # ── Stub if no mrblib ──────────────────────────────────────────────────
  if(NOT EXISTS "${gem_mrblib_c}")
    _gem_func_name("${gem}" gem_fn)
    file(WRITE "${gem_mrblib_c}"
      "/* Auto-generated empty stub */\n"
      "#include <mruby.h>\n"
      "void mrb_load_gem_${gem_fn}(mrb_state *mrb) { (void)mrb; }\n"
    )
  endif()

  # ── Generate init/final stubs for gems without C source ─────────────────
  if(NOT gem_has_src)
    _gem_func_name("${gem}" gem_fn)
    set(_gem_init_c "${MRUBY_GEMS_BUILD_DIR}/${gem}_init.c")
    file(WRITE "${_gem_init_c}"
      "/* Auto-generated stubs for gem without C source */\n"
      "#include <mruby.h>\n"
      "void mrb_${gem_fn}_gem_init(mrb_state *mrb) { (void)mrb; }\n"
      "void mrb_${gem_fn}_gem_final(mrb_state *mrb) { (void)mrb; }\n"
    )
    list(APPEND MRUBY_GEMS_SOURCES "${_gem_init_c}")
    list(APPEND MRUBY_GEM_C_SOURCES "${_gem_init_c}")
    set(gem_has_src TRUE)
    message(STATUS "Gem init stubs: ${gem}")
  endif()

  list(APPEND MRUBY_GEMS_SOURCES "${gem_mrblib_c}")
  list(APPEND MRUBY_GEM_GEN_SOURCES "${gem_mrblib_c}")

  if(gem_has_src)
    list(APPEND MRUBY_GEMS_ACTIVE "${gem}")
  endif()
endforeach()

# ── Generate core mrblib.c (mrb_init_mrblib) ─────────────────────────────────
set(CORE_MRBLIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/mruby/mrblib")
set(CORE_MRBLIB_C "${MRUBY_GEMS_BUILD_DIR}/core_mrblib.c")
if(EXISTS "${CORE_MRBLIB_DIR}")
  file(GLOB _core_rb_files RELATIVE "${CORE_MRBLIB_DIR}" "${CORE_MRBLIB_DIR}/*.rb")
  list(SORT _core_rb_files)
  if(_core_rb_files)
    set(_cbody "")
    string(APPEND _cbody "/* Auto-generated core mrblib - DO NOT EDIT */\n")
    string(APPEND _cbody "#include <mruby/compile.h>\n\n")
    foreach(_rb ${_core_rb_files})
      set(_rb_path "${CORE_MRBLIB_DIR}/${_rb}")
      _write_c_byte_array("${_rb_path}" _c_bytes)
      string(REGEX REPLACE "[^a-zA-Z0-9_]" "_" _var "core_mrblib_${_rb}")
      string(APPEND _cbody "static const unsigned char ${_var}[] = {\n${_c_bytes}\n};\n\n")
    endforeach()
    string(APPEND _cbody "void mrb_init_mrblib(mrb_state *mrb)\n{\n")
    foreach(_rb ${_core_rb_files})
      string(REGEX REPLACE "[^a-zA-Z0-9_]" "_" _var "core_mrblib_${_rb}")
      string(APPEND _cbody "  mrb_load_nstring(mrb, (const char*)${_var}, sizeof(${_var}));\n")
    endforeach()
    string(APPEND _cbody "}\n")
    file(WRITE "${CORE_MRBLIB_C}" "${_cbody}")
    list(APPEND MRUBY_GEMS_SOURCES "${CORE_MRBLIB_C}")
    list(APPEND MRUBY_GEM_GEN_SOURCES "${CORE_MRBLIB_C}")
    message(STATUS "Core mrblib: ${_core_rb_files}")
  endif()
endif()

# ── Generate gems_init.c at CONFIGURE time ──────────────────────────────────
set(GEMS_INIT_C "${MRUBY_GEMS_BUILD_DIR}/gems_init.c")
set(_gi_body "")
string(APPEND _gi_body "/*\n")
string(APPEND _gi_body " * This file contains a list of all\n")
string(APPEND _gi_body " * initializing methods which are\n")
string(APPEND _gi_body " * necessary to bootstrap all gems.\n")
string(APPEND _gi_body " *\n")
string(APPEND _gi_body " * Auto-generated by CMake - DO NOT EDIT\n")
string(APPEND _gi_body " */\n\n")
string(APPEND _gi_body "#include <mruby.h>\n")
string(APPEND _gi_body "#include <mruby/error.h>\n")
string(APPEND _gi_body "#include <mruby/proc.h>\n\n")

# Forward declarations
foreach(gem ${MRUBY_GEMS_ACTIVE})
  _gem_func_name("${gem}" gem_fn)
  string(APPEND _gi_body "void mrb_${gem_fn}_gem_init(mrb_state *mrb);\n")
  string(APPEND _gi_body "void mrb_${gem_fn}_gem_final(mrb_state *mrb);\n")
  string(APPEND _gi_body "void mrb_load_gem_${gem_fn}(mrb_state *mrb);\n")
endforeach()
string(APPEND _gi_body "\n")

# bigint specific
string(APPEND _gi_body "void mrb_mruby_bigint_gem_init(mrb_state *mrb) { (void)mrb; }\n")
string(APPEND _gi_body "void mrb_mruby_bigint_gem_final(mrb_state *mrb) { (void)mrb; }\n")
string(APPEND _gi_body "\n")

# gem_funcs[] array — init, final, and mrblib loader per gem
string(APPEND _gi_body "static const struct {\n")
string(APPEND _gi_body "  void (*init)(mrb_state*);\n")
string(APPEND _gi_body "  void (*final)(mrb_state*);\n")
string(APPEND _gi_body "  void (*loader)(mrb_state*);\n")
string(APPEND _gi_body "} gem_funcs[] = {\n")
foreach(gem ${MRUBY_GEMS_ACTIVE})
  _gem_func_name("${gem}" gem_fn)
  string(APPEND _gi_body "  { mrb_${gem_fn}_gem_init, mrb_${gem_fn}_gem_final, mrb_load_gem_${gem_fn} },\n")
endforeach()
string(APPEND _gi_body "};\n\n")
string(APPEND _gi_body "#define NUM_GEMS ((int)(sizeof(gem_funcs) / sizeof(gem_funcs[0])))\n\n")

# mrb_final_mrbgems helpers
string(APPEND _gi_body "struct final_mrbgems {\n")
string(APPEND _gi_body "  int i;\n")
string(APPEND _gi_body "  int ai;\n")
string(APPEND _gi_body "};\n\n")

string(APPEND _gi_body "static mrb_value\n")
string(APPEND _gi_body "final_mrbgems_body(mrb_state *mrb, void *ud)\n")
string(APPEND _gi_body "{\n")
string(APPEND _gi_body "  struct final_mrbgems *p = (struct final_mrbgems*)ud;\n")
string(APPEND _gi_body "  for (; p->i >= 0; p->i--) {\n")
string(APPEND _gi_body "    gem_funcs[p->i].final(mrb);\n")
string(APPEND _gi_body "    mrb_gc_arena_restore(mrb, p->ai);\n")
string(APPEND _gi_body "  }\n")
string(APPEND _gi_body "  return mrb_nil_value();\n")
string(APPEND _gi_body "}\n\n")

string(APPEND _gi_body "static void\n")
string(APPEND _gi_body "mrb_final_mrbgems(mrb_state *mrb)\n")
string(APPEND _gi_body "{\n")
string(APPEND _gi_body "  struct final_mrbgems a = { NUM_GEMS - 1, mrb_gc_arena_save(mrb) };\n")
string(APPEND _gi_body "  for (; a.i >= 0; a.i--) {\n")
string(APPEND _gi_body "    mrb_protect_error(mrb, final_mrbgems_body, &a, NULL);\n")
string(APPEND _gi_body "    mrb_gc_arena_restore(mrb, a.ai);\n")
string(APPEND _gi_body "  }\n")
string(APPEND _gi_body "}\n\n")

# mrb_init_mrbgems
string(APPEND _gi_body "void\n")
string(APPEND _gi_body "mrb_init_mrbgems(mrb_state *mrb)\n")
string(APPEND _gi_body "{\n")
string(APPEND _gi_body "  int ai = mrb_gc_arena_save(mrb);\n")
string(APPEND _gi_body "  for (int i = 0; i < NUM_GEMS; i++) {\n")
string(APPEND _gi_body "    gem_funcs[i].init(mrb);\n")
string(APPEND _gi_body "    gem_funcs[i].loader(mrb);\n")
string(APPEND _gi_body "    mrb_gc_arena_restore(mrb, ai);\n")
string(APPEND _gi_body "    mrb_vm_ci_env_clear(mrb, mrb->c->cibase);\n")
string(APPEND _gi_body "    if (mrb->exc) {\n")
string(APPEND _gi_body "      mrb_p(mrb, mrb_format(\"Gem initialize error (%d/%d)\", i, NUM_GEMS));\n")
string(APPEND _gi_body "      mrb_exc_raise(mrb, mrb_obj_value(mrb->exc));\n")
string(APPEND _gi_body "    }\n")
string(APPEND _gi_body "  }\n")
string(APPEND _gi_body "  mrb_state_atexit(mrb, mrb_final_mrbgems);\n")
string(APPEND _gi_body "}\n")

file(WRITE "${GEMS_INIT_C}" "${_gi_body}")
message(STATUS "Generated gems_init.c (${MRUBY_GEMS_ACTIVE})")

list(APPEND MRUBY_GEMS_SOURCES "${GEMS_INIT_C}")
list(APPEND MRUBY_GEM_GEN_SOURCES "${GEMS_INIT_C}")

# ── Add gem C sources to mruby target (for presym scanning) ──────────────────
target_sources(mruby PRIVATE ${MRUBY_GEM_C_SOURCES})
target_include_directories(mruby PRIVATE
  "${MRUBY_GEMS_BUILD_DIR}"   # for generated gem C files
)

# ═══════════════════════════════════════════════════════════════════════════════
# Presym scanning: extract all <@! "symbol" !@> markers from preprocessed sources
# ═══════════════════════════════════════════════════════════════════════════════

find_package(Python3 REQUIRED COMPONENTS Interpreter)

# ── Include paths needed for preprocessing ───────────────────────────────────
set(MRUBY_PP_INCLUDE_DIRS
  "${CMAKE_CURRENT_SOURCE_DIR}/mruby/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/mruby/src"
)

# Add gem-specific include directories for presym preprocessing
foreach(gem ${MRUBY_GEMS})
  set(gem_dir "${MRUBY_GEMS_DIR}/${gem}")
  if(EXISTS "${gem_dir}/include")
    list(APPEND MRUBY_PP_INCLUDE_DIRS "${gem_dir}/include")
  endif()
  # Some gems include local headers from their src/ directory
  if(EXISTS "${gem_dir}/src")
    list(APPEND MRUBY_PP_INCLUDE_DIRS "${gem_dir}/src")
  endif()
endforeach()

# Add external gem include directories for presym preprocessing
foreach(gem ${EXTERNAL_MRUBY_GEMS})
  set(gem_dir "${EXTERNAL_MRUBY_GEMS_DIR}/${gem}")
  if(EXISTS "${gem_dir}/include")
    list(APPEND MRUBY_PP_INCLUDE_DIRS "${gem_dir}/include")
  endif()
  if(EXISTS "${gem_dir}/src")
    list(APPEND MRUBY_PP_INCLUDE_DIRS "${gem_dir}/src")
  endif()
endforeach()

# mruby-compiler gem needs prism_parser headers
# Try common prism include locations
if(EXISTS "${PRISM_SOURCE_DIR}")
  get_target_property(PRISM_INCLUDES prism INCLUDE_DIRECTORIES)
  list(APPEND MRUBY_PP_INCLUDE_DIRS "${PRISM_INCLUDES}")
endif()

if(EXISTS "${ONIG_SOURCE_DIR}")
  get_target_property(ONIG_INCLUDES onig INCLUDE_DIRECTORIES)
  list(APPEND MRUBY_PP_INCLUDE_DIRS "${ONIG_INCLUDES}")
endif()

# ── Build include flags as a list ────────────────────────────────────────────
set(MRUBY_PP_INCLUDE_FLAGS "")
foreach(dir ${MRUBY_PP_INCLUDE_DIRS})
  if(MSVC)
    list(APPEND MRUBY_PP_INCLUDE_FLAGS "/I${dir}")
  else()
    list(APPEND MRUBY_PP_INCLUDE_FLAGS "-I${dir}")
  endif()
endforeach()

# ── Generate .pi file for each mruby source ──────────────────────────────────
# Uses a Python wrapper (presym_pp.py) to invoke the compiler preprocessor.
# This avoids all argument-quoting issues with CMake's add_custom_command
# on MSVC (where /Fi flag has notoriously brittle quoting semantics).
get_target_property(PRESYM_MRUBY_SOURCES mruby SOURCES)

set(PI_DIR "${CMAKE_CURRENT_BINARY_DIR}/presym_pi")
file(MAKE_DIRECTORY ${PI_DIR})

set(PRESYM_PP_PY "${CMAKE_CURRENT_SOURCE_DIR}/presym_pp.py")
set(PI_FILES "")

foreach(src ${PRESYM_MRUBY_SOURCES})
  # Use relative path as .pi name to avoid collisions (gems + core share filenames)
  cmake_path(RELATIVE_PATH src BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" OUTPUT_VARIABLE src_rel)
  string(REGEX REPLACE "[/\\]" "_" pi_name "${src_rel}")
  string(REGEX REPLACE "\\.c$" "" pi_name "${pi_name}")
  set(pi_file "${PI_DIR}/${pi_name}.pi")
  list(APPEND PI_FILES "${pi_file}")

  add_custom_command(
    OUTPUT "${pi_file}"
    COMMAND "${Python3_EXECUTABLE}" "${PRESYM_PP_PY}"
            --compiler "${CMAKE_C_COMPILER}"
            --output "${pi_file}"
            --source "${src}"
            -- ${MRUBY_PP_INCLUDE_FLAGS}
    DEPENDS "${src}" "${PRESYM_PP_PY}"
    COMMENT "Presym preprocessing: ${pi_name} -> ${pi_name}.pi"
  )
endforeach()

# ── Scan all .pi files, export symbol list ───────────────────────────────────
set(PRESYM_LIST "${CMAKE_CURRENT_BINARY_DIR}/presym.list")
set(PRESYM_SCAN_PY "${CMAKE_CURRENT_SOURCE_DIR}/presym_scan.py")

add_custom_command(
  OUTPUT "${PRESYM_LIST}"
  COMMAND "${Python3_EXECUTABLE}" "${PRESYM_SCAN_PY}"
          list
          --verbose
          --output "${PRESYM_LIST}"
          ${PI_FILES}
  DEPENDS ${PI_FILES} "${PRESYM_SCAN_PY}"
  COMMENT "Scanning presym markers -> presym.list"
)

# ── Generate id.h and table.h from presym.list ───────────────────────────────
set(ID_H   "${PRESYM_HEADER_DIR}/id.h")
set(TABLE_H "${PRESYM_HEADER_DIR}/table.h")

add_custom_command(
  OUTPUT "${ID_H}" "${TABLE_H}"
  COMMAND "${Python3_EXECUTABLE}" "${PRESYM_SCAN_PY}"
          headers
          --input-list "${PRESYM_LIST}"
          --output-dir "${PRESYM_HEADER_DIR}"
  DEPENDS "${PRESYM_LIST}" "${PRESYM_SCAN_PY}"
  COMMENT "Generating presym id.h and table.h"
)

# ── Convenience targets ──────────────────────────────────────────────────────
add_custom_target(mruby_presym_list DEPENDS "${PRESYM_LIST}")
add_custom_target(mruby_presym_headers DEPENDS "${ID_H}" "${TABLE_H}")

# ── Hook: mruby compilation depends on generated presym headers ──────────────
add_dependencies(mruby mruby_presym_headers)

# ── Add generated gem sources to mruby (after presym, they don't need scanning)
target_sources(mruby PRIVATE ${MRUBY_GEM_GEN_SOURCES})
