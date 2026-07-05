# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

cmake_minimum_required(VERSION 3.30...4.3)

include(infra/cmake/enable-experimental-import-std.cmake)
include(infra/cmake/CPM.cmake)

project(
    beman.sendosio
    DESCRIPTION "An experiment in \"porting\" Capy + Corosio to senders"
    LANGUAGES CXX
    VERSION 0.1.0
)

# [CMAKE.SKIP_TESTS]
option(
    BEMAN_SENDOSIO_BUILD_TESTS
    "Enable building tests and test infrastructure. Default: ${PROJECT_IS_TOP_LEVEL}. Values: { ON, OFF }."
    ${PROJECT_IS_TOP_LEVEL}
)

# [CMAKE.SKIP_EXAMPLES]
option(
    BEMAN_SENDOSIO_BUILD_EXAMPLES
    "Enable building examples. Default: ${PROJECT_IS_TOP_LEVEL}. Values: { ON, OFF }."
    ${PROJECT_IS_TOP_LEVEL}
)

# Keep a compile_commands.json symlink at the project root pointing at
# whichever preset was most recently configured, so clangd (and other
# tooling that expects the database at the source root) can find it
# without per-preset configuration. Only do this for the top-level
# build -- not when this project is pulled in as a dependency -- and
# only when a compilation database is actually being generated.
if(PROJECT_IS_TOP_LEVEL AND CMAKE_EXPORT_COMPILE_COMMANDS)
    file(
        CREATE_LINK
            "${PROJECT_BINARY_DIR}/compile_commands.json"
            "${PROJECT_SOURCE_DIR}/compile_commands.json"
        SYMBOLIC
        COPY_ON_ERROR
    )
endif()

option(BEMAN_SENDOSIO_USE_MODULES "Provide beman.sendosio as a C++ module" OFF)

if(BEMAN_SENDOSIO_USE_MODULES)
    set(CMAKE_CXX_SCAN_FOR_MODULES ON)
endif()

option(
    BEMAN_SENDOSIO_FORCE_NO_CONTRACTS
    "Forcibly disable use of C++26 contracts even if they're supported. Default: ON. Values: { ON, OFF }."
    ON
)

configure_file(
    "${PROJECT_SOURCE_DIR}/include/beman/sendosio/config_generated.hpp.in"
    "${PROJECT_BINARY_DIR}/include/beman/sendosio/config_generated.hpp"
    @ONLY
)

# for find of beman_install_library and configure_build_telemetry
include(infra/cmake/beman-install-library.cmake)
include(infra/cmake/BuildTelemetryConfig.cmake)

cpmaddpackage("gh:NVIDIA/stdexec#main")

if(BEMAN_SENDOSIO_USE_MODULES)
    add_library(beman.sendosio STATIC)
    target_link_libraries(beman.sendosio PRIVATE STDEXEC::stdexec)
else()
    add_library(beman.sendosio INTERFACE)
    target_link_libraries(
        beman.sendosio
        INTERFACE $<BUILD_INTERFACE:STDEXEC::stdexec>
    )
endif()
add_library(beman::sendosio ALIAS beman.sendosio)

if(BEMAN_SENDOSIO_USE_MODULES)
    target_sources(
        beman.sendosio
        PUBLIC
            FILE_SET CXX_MODULES
            FILE_SET HEADERS
                BASE_DIRS
                    "${CMAKE_CURRENT_SOURCE_DIR}/include"
                    "${CMAKE_CURRENT_BINARY_DIR}/include"
    )
    set_target_properties(beman.sendosio PROPERTIES CXX_MODULE_STD ON)
    target_compile_features(beman.sendosio PUBLIC cxx_std_23)
else()
    target_sources(
        beman.sendosio
        PUBLIC
            FILE_SET HEADERS
                BASE_DIRS
                    "${CMAKE_CURRENT_SOURCE_DIR}/include"
                    "${CMAKE_CURRENT_BINARY_DIR}/include"
    )
    set_target_properties(
        beman.sendosio
        PROPERTIES VERIFY_INTERFACE_HEADER_SETS ${PROJECT_IS_TOP_LEVEL}
    )
endif()

add_subdirectory(include/beman/sendosio)

beman_install_library(beman.sendosio TARGETS beman.sendosio)
configure_build_telemetry()

if(BEMAN_SENDOSIO_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests/beman/sendosio)
endif()

if(BEMAN_SENDOSIO_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
