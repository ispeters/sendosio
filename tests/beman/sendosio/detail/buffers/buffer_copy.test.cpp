// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/sendosio/config.hpp>

#include <catch2/catch_all.hpp>

#include <beman/sendosio/detail/buffers/buffer_copy.hpp>
#include <beman/sendosio/detail/buffers/make_buffer.hpp>

#if BEMAN_SENDOSIO_USE_MODULES()
import std;
#else
    #include <array>
    #include <ranges>
    #include <string_view>
#endif

namespace {

namespace sendosio = ::beman::sendosio;

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

TEST_CASE("buffer_copy can handle copying from a range of one-byte buffers",
          "[sendosio::buffer_copy]") {
    constexpr char                       source[] = "hello, world!";
    std::array<char, sizeof(source) - 1> dest{};

    auto source_buffers = source | std::views::take(dest.size()) |
                          std::views::transform([](const char& c) noexcept {
                              return sendosio::const_buffer(&c, 1);
                          });
    auto dest_buffers   = sendosio::make_buffer(dest);

    const auto bytes = sendosio::buffer_copy(dest_buffers, source_buffers);

    REQUIRE(bytes == dest.size());
    REQUIRE(std::string_view(dest.data(), dest.size()) == source);
}

TEST_CASE("buffer_copy can handle the Cartesian product of possible two-part weak "
          "compositions of the source and destination storage",
          "[sendosio::buffer_copy]") {
    constexpr char                       source[] = "hello, world!";
    std::array<char, sizeof(source) - 1> dest{};

    for (std::size_t src_split = 0; src_split <= dest.size(); ++src_split) {
        const std::array<sendosio::const_buffer, 2> src_buffers{
            sendosio::make_buffer(source, src_split),
            sendosio::make_buffer(source) + src_split};

        for (std::size_t dest_split = 0; dest_split <= dest.size(); ++dest_split) {
            const std::array<sendosio::mutable_buffer, 2> dest_buffers{
                sendosio::make_buffer(dest, dest_split),
                sendosio::make_buffer(dest) + dest_split};

            dest             = {};
            const auto bytes = sendosio::buffer_copy(dest_buffers, src_buffers);

            REQUIRE(bytes == dest.size());
            REQUIRE(std::string_view(dest.data(), dest.size()) == source);
        }
    }
}

TEST_CASE("buffer_copy can handle the Cartesian product of possible three-part weak "
          "compositions of the source and destination storage",
          "[sendosio::buffer_copy]") {
    constexpr char                       source[] = "hello, world!";
    std::array<char, sizeof(source) - 1> dest{};

    for (std::size_t src_split = 0; src_split <= dest.size(); ++src_split) {
        const auto shead = sendosio::make_buffer(source, src_split);
        const auto stail = sendosio::make_buffer(source) + src_split;

        for (std::size_t src_split2 = 0; src_split2 <= stail.size(); ++src_split2) {
            const std::array<sendosio::const_buffer, 3> src_buffers{
                shead, sendosio::make_buffer(stail, src_split2), stail + src_split2};

            for (std::size_t dest_split = 0; dest_split <= dest.size(); ++dest_split) {
                const auto dhead = sendosio::make_buffer(dest, dest_split);
                const auto dtail = sendosio::make_buffer(dest) + dest_split;

                for (std::size_t dest_split2 = 0; dest_split2 <= dtail.size();
                     ++dest_split2) {
                    const std::array<sendosio::mutable_buffer, 3> dest_buffers{
                        dhead,
                        sendosio::make_buffer(dtail, dest_split2),
                        dtail + dest_split2};

                    dest             = {};
                    const auto bytes = sendosio::buffer_copy(dest_buffers, src_buffers);

                    REQUIRE(bytes == dest.size());
                    REQUIRE(std::string_view(dest.data(), dest.size()) == source);
                }
            }
        }
    }
}

} // namespace
