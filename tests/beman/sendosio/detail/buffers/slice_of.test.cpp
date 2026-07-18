// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "beman/sendosio/detail/buffers/slice_of.hpp"

#include <catch2/catch_all.hpp>

namespace {

namespace sendosio = beman::sendosio;

TEST_CASE("slice_of.hpp is self-contained", "[sendosio::slice_of]") {
    // this test file really only proves that slice_of.hpp is self-contained; the
    // behaviour of a slice_of<T> is validated in buffer_slice.test.cpp
    STATIC_REQUIRE(true);
}

} // namespace
