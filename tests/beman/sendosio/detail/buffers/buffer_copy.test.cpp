// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "beman/sendosio/detail/buffers/buffer_copy.hpp"

#include <catch2/catch_all.hpp>

namespace {

namespace sendosio = beman::sendosio;

TEST_CASE("buffer_copy.hpp is self-contained", "[sendosio::buffer_copy]") {
    STATIC_REQUIRE(sizeof(sendosio::buffer_copy_t) > 0);
}

} // namespace
