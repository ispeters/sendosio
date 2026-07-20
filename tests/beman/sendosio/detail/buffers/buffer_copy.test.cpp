// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "beman/sendosio/detail/buffers/buffer_copy.hpp"

#include <catch2/catch_all.hpp>

#include <beman/sendosio/detail/buffers/make_buffer.hpp>

#include <array>
#include <string_view>

namespace {

namespace sendosio = beman::sendosio;

TEST_CASE("buffer_copy can copy one string to another", "[sendosio::buffer_copy]") {
    constexpr char                       source[] = "hello, world!";
    std::array<char, sizeof(source) - 1> dest{};

    const auto bytes =
        sendosio::buffer_copy(sendosio::make_buffer(dest), sendosio::make_buffer(source));

    REQUIRE(bytes == dest.size());
    REQUIRE(std::string_view(dest.data(), dest.size()) == source);
}

} // namespace
