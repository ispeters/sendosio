// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "beman/sendosio/detail/buffers.hpp"

#include <catch2/catch_all.hpp>

#include <concepts>

namespace {

namespace sendosio = beman::sendosio;

TEST_CASE("a const_buffer is a const_buffer_sequence", "[sendosio::buffers]") {
    STATIC_REQUIRE(sendosio::const_buffer_sequence<sendosio::const_buffer>);
}

} // namespace
