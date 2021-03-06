# Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
# This file is subject to the license terms in the LICENSE file
# found in the top-level directory of this distribution.

set(include_dir ${CMAKE_CURRENT_SOURCE_DIR}/../include/lexy)
set(header_files
        ${include_dir}/_detail/ascii_table.hpp
        ${include_dir}/_detail/assert.hpp
        ${include_dir}/_detail/buffer_builder.hpp
        ${include_dir}/_detail/config.hpp
        ${include_dir}/_detail/detect.hpp
        ${include_dir}/_detail/integer_sequence.hpp
        ${include_dir}/_detail/invoke.hpp
        ${include_dir}/_detail/iterator.hpp
        ${include_dir}/_detail/lazy_init.hpp
        ${include_dir}/_detail/memory_resource.hpp
        ${include_dir}/_detail/nttp_string.hpp
        ${include_dir}/_detail/stateless_lambda.hpp
        ${include_dir}/_detail/std.hpp
        ${include_dir}/_detail/string_view.hpp
        ${include_dir}/_detail/type_name.hpp

        ${include_dir}/dsl/alternative.hpp
        ${include_dir}/dsl/any.hpp
        ${include_dir}/dsl/ascii.hpp
        ${include_dir}/dsl/base.hpp
        ${include_dir}/dsl/bom.hpp
        ${include_dir}/dsl/brackets.hpp
        ${include_dir}/dsl/branch.hpp
        ${include_dir}/dsl/capture.hpp
        ${include_dir}/dsl/choice.hpp
        ${include_dir}/dsl/code_point.hpp
        ${include_dir}/dsl/combination.hpp
        ${include_dir}/dsl/context.hpp
        ${include_dir}/dsl/context_counter.hpp
        ${include_dir}/dsl/context_flag.hpp
        ${include_dir}/dsl/context_lexeme.hpp
        ${include_dir}/dsl/delimited.hpp
        ${include_dir}/dsl/digit.hpp
        ${include_dir}/dsl/eof.hpp
        ${include_dir}/dsl/encode.hpp
        ${include_dir}/dsl/error.hpp
        ${include_dir}/dsl/if.hpp
        ${include_dir}/dsl/integer.hpp
        ${include_dir}/dsl/label.hpp
        ${include_dir}/dsl/lookahead.hpp
        ${include_dir}/dsl/loop.hpp
        ${include_dir}/dsl/member.hpp
        ${include_dir}/dsl/minus.hpp
        ${include_dir}/dsl/newline.hpp
        ${include_dir}/dsl/option.hpp
        ${include_dir}/dsl/peek.hpp
        ${include_dir}/dsl/position.hpp
        ${include_dir}/dsl/production.hpp
        ${include_dir}/dsl/punctuator.hpp
        ${include_dir}/dsl/return.hpp
        ${include_dir}/dsl/separator.hpp
        ${include_dir}/dsl/sequence.hpp
        ${include_dir}/dsl/sign.hpp
        ${include_dir}/dsl/switch.hpp
        ${include_dir}/dsl/terminator.hpp
        ${include_dir}/dsl/times.hpp
        ${include_dir}/dsl/token.hpp
        ${include_dir}/dsl/until.hpp
        ${include_dir}/dsl/value.hpp
        ${include_dir}/dsl/while.hpp
        ${include_dir}/dsl/whitespace.hpp

        ${include_dir}/engine/any.hpp
        ${include_dir}/engine/base.hpp
        ${include_dir}/engine/char_class.hpp
        ${include_dir}/engine/code_point.hpp
        ${include_dir}/engine/digits.hpp
        ${include_dir}/engine/eof.hpp
        ${include_dir}/engine/failure.hpp
        ${include_dir}/engine/literal.hpp
        ${include_dir}/engine/lookahead.hpp
        ${include_dir}/engine/minus.hpp
        ${include_dir}/engine/trie.hpp
        ${include_dir}/engine/until.hpp
        ${include_dir}/engine/while.hpp

        ${include_dir}/input/argv_input.hpp
        ${include_dir}/input/base.hpp
        ${include_dir}/input/buffer.hpp
        ${include_dir}/input/file.hpp
        ${include_dir}/input/null_input.hpp
        ${include_dir}/input/range_input.hpp
        ${include_dir}/input/shell.hpp
        ${include_dir}/input/string_input.hpp

        ${include_dir}/callback.hpp
        ${include_dir}/dsl.hpp
        ${include_dir}/encoding.hpp
        ${include_dir}/error.hpp
        ${include_dir}/lexeme.hpp
        ${include_dir}/match.hpp
        ${include_dir}/parse.hpp
        ${include_dir}/parse_tree.hpp
        ${include_dir}/production.hpp
        ${include_dir}/result.hpp
        ${include_dir}/token.hpp
        ${include_dir}/validate.hpp)

# Base target for common options.
add_library(_lexy_base INTERFACE)
target_sources(_lexy_base INTERFACE ${header_files})
if ("cxx_std_20" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
    target_compile_features(_lexy_base INTERFACE cxx_std_20)
else()
    target_compile_features(_lexy_base INTERFACE cxx_std_17)
endif()

if (LEXY_USER_CONFIG_HEADER)
    if(EXISTS ${LEXY_USER_CONFIG_HEADER})
        target_compile_definitions(_lexy_base INTERFACE LEXY_USER_CONFIG_HEADER="${LEXY_USER_CONFIG_HEADER}")
    else()
        message(WARNING "User config header for lexy specified as '${LEXY_USER_CONFIG_HEADER}', but not found.")
    endif()
endif()

# Core library.
add_library(lexy_core INTERFACE)
add_library(foonathan::lexy::core ALIAS lexy_core)
target_link_libraries(lexy_core INTERFACE _lexy_base)
target_include_directories(lexy_core SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

# Core library with warnings; for development only.
add_library(lexy_dev INTERFACE)
add_library(foonathan::lexy::dev ALIAS lexy_dev)
target_link_libraries(lexy_dev INTERFACE _lexy_base)
target_include_directories(lexy_dev INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    if("${CMAKE_CXX_SIMULATE_ID}" STREQUAL "MSVC")
        target_compile_options(lexy_dev INTERFACE /WX /W3 /D _CRT_SECURE_NO_WARNINGS)
    else()
        target_compile_options(lexy_dev INTERFACE -pedantic-errors -Werror -Wall -Wextra -Wconversion -Wsign-conversion)
    endif()
    # clang doesn't like operator prcedence rules we're using for the DSL.
    target_compile_options(lexy_dev INTERFACE -Wno-shift-op-parentheses)
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    target_compile_options(lexy_dev INTERFACE -pedantic-errors -Werror -Wall -Wextra -Wconversion -Wsign-conversion)
    # GCC doesn't like the operator precedence rules we're using for the DSL.
    target_compile_options(lexy_dev INTERFACE -Wno-parentheses)
    # GCC doesn't properly detect when local typedefs escape via the return type.
    target_compile_options(lexy_dev INTERFACE -Wno-unused-local-typedefs)
    # GCC's arry bounds warning seems to have false positives.
    target_compile_options(lexy_dev INTERFACE -Wno-array-bounds)
elseif(MSVC)
    target_compile_options(lexy_dev INTERFACE /WX /W3 /D _CRT_SECURE_NO_WARNINGS /wd5105)
endif()

# Link to have FILE I/O.
add_library(lexy_file)
add_library(foonathan::lexy::file ALIAS lexy_file)
target_link_libraries(lexy_file PRIVATE foonathan::lexy::dev)
target_sources(lexy_file PRIVATE input/file.cpp)

# Umbrella target with all components.
add_library(lexy INTERFACE)
add_library(foonathan::lexy ALIAS lexy)
target_link_libraries(lexy INTERFACE foonathan::lexy::core foonathan::lexy::file)

