// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "beman/sendosio/detail/buffers/consuming_buffers.hpp"

#include <catch2/catch_all.hpp>

#include <beman/sendosio/detail/buffers.hpp>
#include <beman/sendosio/detail/buffers/make_buffer.hpp>
#include <beman/sendosio/detail/contracts.hpp>

#include <array>
#include <concepts>
#include <limits>
#include <ranges>
#include <type_traits>

namespace {

// TODO: this file probably needs more test cases; for one thing, it would be a good idea
//       to confirm that different instances of consuming_buffers constructed from the
//       same buffer sequence are independent

namespace sendosio = beman::sendosio;

template <std::invocable<sendosio::mutable_buffer, char*> Predicate>
    requires std::
        same_as<bool, std::invoke_result_t<Predicate&, sendosio::mutable_buffer, char*>>
    constexpr bool validate_predicate_over_nonempty_buffer(Predicate pred) noexcept {
    char  message[] = "hello, world!";
    char* ptr       = message;
    return pred(sendosio::mutable_buffer(ptr, sizeof(message)), ptr);
}

TEMPLATE_TEST_CASE("consuming_buffers(...).consume(prefix) handles all contingencies",
                   "[sendosio::consuming_buffers]",
                   sendosio::const_buffer,
                   sendosio::mutable_buffer) {

    STATIC_REQUIRE(
        validate_predicate_over_nonempty_buffer([](const TestType buffer, auto) noexcept {
            const auto about_half = buffer.size() / 2;

            auto slice = sendosio::consuming_buffers(buffer);

            slice.consume(about_half);

            auto data = slice.data();

            return std::ranges::equal(data, std::views::single(buffer + about_half));
        }));

    STATIC_REQUIRE(
        validate_predicate_over_nonempty_buffer([](const TestType buffer, auto) noexcept {
            auto truncatedBuffer = sendosio::make_buffer(buffer, 2);
            auto slice           = sendosio::consuming_buffers(truncatedBuffer);

            slice.consume(1);

            auto data = slice.data();

            auto expected = sendosio::make_buffer(buffer, 2) + 1;

            return std::ranges::equal(data, std::views::single(expected));
        }));

    STATIC_REQUIRE(
        validate_predicate_over_nonempty_buffer([](const TestType buffer, auto) noexcept {
            auto truncatedBuffer = sendosio::make_buffer(buffer, 2);
            auto slice           = sendosio::consuming_buffers(truncatedBuffer);

            slice.consume(3);

            auto data = slice.data();

            return std::ranges::empty(data);
        }));
}

} // namespace
