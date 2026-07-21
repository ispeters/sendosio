// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "beman/sendosio/detail/buffers/buffer_copy.hpp"

#include <catch2/catch_all.hpp>

#include <beman/sendosio/detail/buffers/make_buffer.hpp>

#include <array>
#include <ranges>
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

TEST_CASE("buffer_copy can handle mirrored ranges of one-byte buffers",
          "[sendosio::buffer_copy]") {
    constexpr char                       source[] = "hello, world!";
    std::array<char, sizeof(source) - 1> dest{};

    auto source_buffers = source | std::views::take(dest.size()) |
                          std::views::transform([](const char& c) noexcept {
                              return sendosio::const_buffer(&c, 1);
                          });
    auto dest_buffers   = dest | std::views::transform([](char& c) noexcept {
                            return sendosio::mutable_buffer(&c, 1);
                          });

    const auto bytes = sendosio::buffer_copy(dest_buffers, source_buffers);

    REQUIRE(bytes == dest.size());
    REQUIRE(std::string_view(dest.data(), dest.size()) == source);
}

TEST_CASE("buffer_copy can handle copying into a range of one-byte buffers",
          "[sendosio::buffer_copy]") {
    constexpr char                       source[] = "hello, world!";
    std::array<char, sizeof(source) - 1> dest{};

    auto source_buffers = sendosio::make_buffer(source);
    auto dest_buffers   = dest | std::views::transform([](char& c) noexcept {
                            return sendosio::mutable_buffer(&c, 1);
                          });

    const auto bytes = sendosio::buffer_copy(dest_buffers, source_buffers);

    REQUIRE(bytes == dest.size());
    REQUIRE(std::string_view(dest.data(), dest.size()) == source);
}

} // namespace
