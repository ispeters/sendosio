# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

find_package(Catch2 3 REQUIRED)

add_executable(beman.sendosio.tests.detail.buffers)
target_sources(
    beman.sendosio.tests.detail.buffers
    PRIVATE
        detail/buffer_algorithms.test.cpp
        detail/buffer_concepts.test.cpp
        detail/buffers.test.cpp
        detail/buffers/buffer_copy.test.cpp
        detail/buffers/buffer_slice.test.cpp
        detail/buffers/consuming_buffers.test.cpp
        detail/buffers/make_buffer.test.cpp
        detail/buffers/slice_of.test.cpp
        detail/concept/read_stream.test.cpp
        detail/read.test.cpp
)
target_link_libraries(
    beman.sendosio.tests.detail.buffers
    PRIVATE beman::sendosio Catch2::Catch2WithMain STDEXEC::stdexec
)
if(BEMAN_SENDOSIO_USE_MODULES)
    set_target_properties(
        beman.sendosio.tests.detail.buffers
        PROPERTIES CXX_MODULE_STD ON
    )
endif()

include(Catch)
catch_discover_tests(beman.sendosio.tests.detail.buffers)
