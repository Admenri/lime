cmake_minimum_required(VERSION 3.20)

set(AUTOGEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/gen")
set(TEMPLATE_DIR "${PRISM_SOURCE_DIR}/templates")
file(MAKE_DIRECTORY ${AUTOGEN_DIR})

set(AUTOGEN_LIST
  "include/prism/ast.h"
  "include/prism/diagnostic.h"
  "src/diagnostic.c"
  "src/node.c"
  "src/prettyprint.c"
  "src/serialize.c"
  "src/token_type.c"
)

foreach(target_erb IN LISTS AUTOGEN_LIST)
  execute_process(
    COMMAND ruby "${TEMPLATE_DIR}/template.rb" "${target_erb}" "${AUTOGEN_DIR}/${target_erb}"
    WORKING_DIRECTORY "${AUTOGEN_DIR}"
    RESULT_VARIABLE erb_result
  )
  message(STATUS "Generating Prism Templates: ${target_erb} - Result: ${erb_result}")
endforeach()

message(STATUS "Prism Include: ${PRISM_SOURCE_DIR}/include")
message(STATUS "Prism Include: ${AUTOGEN_DIR}/include")

add_library(prism STATIC)
target_include_directories(prism PUBLIC
  ${PRISM_SOURCE_DIR}/include
  ${AUTOGEN_DIR}/include
)
target_sources(prism PUBLIC
  ${PRISM_SOURCE_DIR}/src/util/pm_buffer.c
  ${PRISM_SOURCE_DIR}/src/util/pm_char.c
  ${PRISM_SOURCE_DIR}/src/util/pm_constant_pool.c
  ${PRISM_SOURCE_DIR}/src/util/pm_integer.c
  ${PRISM_SOURCE_DIR}/src/util/pm_list.c
  ${PRISM_SOURCE_DIR}/src/util/pm_memchr.c
  ${PRISM_SOURCE_DIR}/src/util/pm_newline_list.c
  ${PRISM_SOURCE_DIR}/src/util/pm_string.c
  ${PRISM_SOURCE_DIR}/src/util/pm_strncasecmp.c
  ${PRISM_SOURCE_DIR}/src/util/pm_strpbrk.c
  ${PRISM_SOURCE_DIR}/src/encoding.c
  ${PRISM_SOURCE_DIR}/src/options.c
  ${PRISM_SOURCE_DIR}/src/pack.c
  ${PRISM_SOURCE_DIR}/src/prism.c
  ${PRISM_SOURCE_DIR}/src/regexp.c
  ${PRISM_SOURCE_DIR}/src/static_literals.c
  ${AUTOGEN_DIR}/src/diagnostic.c
  ${AUTOGEN_DIR}/src/node.c
  ${AUTOGEN_DIR}/src/prettyprint.c
  ${AUTOGEN_DIR}/src/serialize.c
  ${AUTOGEN_DIR}/src/token_type.c
)
