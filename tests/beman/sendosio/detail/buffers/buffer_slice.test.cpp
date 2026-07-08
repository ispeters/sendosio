// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "beman/sendosio/detail/buffers/buffer_slice.hpp"

#include <catch2/catch_all.hpp>

#include <beman/sendosio/detail/buffers.hpp>
#include <beman/sendosio/detail/buffers/make_buffer.hpp>
#include <beman/sendosio/detail/contracts.hpp>

#include <array>

namespace {

namespace sendosio = beman::sendosio;

template <std::invocable<sendosio::mutable_buffer, char*> Predicate>
    requires std::
        same_as<bool, std::invoke_result_t<Predicate&, sendosio::mutable_buffer, char*>>
    constexpr bool validate_predicate_over_nonempty_buffer(Predicate pred) noexcept {
    char  message[] = "hello, world!";
    char* ptr       = message;
    return pred(sendosio::mutable_buffer(ptr, sizeof(message)), ptr);
}

TEMPLATE_TEST_CASE("buffer_slice(buffer) is the identity",
                   "[sendosio::buffer_slice]",
                   sendosio::const_buffer,
                   sendosio::mutable_buffer) {
    STATIC_REQUIRE(validate_predicate_over_nonempty_buffer([](const TestType buffer,
                                                              auto) noexcept {
        auto slice = sendosio::buffer_slice(buffer);

        auto data = slice.data();

        static_assert(std::same_as<TestType, std::ranges::range_value_t<decltype(data)>>);

        // this asserts that data contains one buffer that is equal to buffer, i.e.
        // that buffer_slice(buffer) is the identity
        return std::ranges::equal(data, std::views::single(buffer));
    }));
}

TEMPLATE_TEST_CASE("buffer_slice(buffer, offset) removes a prefix from the given buffer",
                   "[sendosio::buffer_slice]",
                   sendosio::const_buffer,
                   sendosio::mutable_buffer) {
    STATIC_REQUIRE(validate_predicate_over_nonempty_buffer(
        [](const TestType buffer, char* message) noexcept {
            constexpr std::size_t offset = 7; // skip past "hello, "

            auto slice = sendosio::buffer_slice(buffer, offset);

            auto data = slice.data();

            // slice and data point into buffer so, to validate that data is correct, we
            // can't touch buffer; instead, make a copy, mutate it, and compare to the
            // result
            auto buffer_copy = buffer;
            buffer_copy += offset;

            BEMAN_SENDOSIO_CONTRACT_ASSERT(buffer_copy.data() == (message + offset));

            return std::ranges::equal(data, std::views::single(buffer_copy));
        }));

    STATIC_REQUIRE(
        validate_predicate_over_nonempty_buffer([](const TestType buffer, auto) noexcept {
            // remove exactly the whole buffer
            auto slice = sendosio::buffer_slice(buffer, buffer.size());

            auto data = slice.data();

            // there's no data left in the slice so the range is empty
            return std::ranges::empty(data);
        }));

    STATIC_REQUIRE(
        validate_predicate_over_nonempty_buffer([](const TestType buffer, auto) noexcept {
            // remove the most it is possible to remove
            auto slice =
                sendosio::buffer_slice(buffer, (std::numeric_limits<std::size_t>::max)());

            auto data = slice.data();

            // the result of offsetting "into infinity" is still just an empty range
            return std::ranges::empty(data);
        }));
}

TEMPLATE_TEST_CASE("buffer_slice(buffer, 0, length) limits the given buffer to a prefix",
                   "[sendosio::buffer_slice]",
                   sendosio::const_buffer,
                   sendosio::mutable_buffer) {
    STATIC_REQUIRE(validate_predicate_over_nonempty_buffer([](const TestType buffer,
                                                              char* message) noexcept {
        constexpr std::size_t length = 5; // limit to "hello"

        BEMAN_SENDOSIO_CONTRACT_ASSERT(buffer.size() > length);

        auto slice = sendosio::buffer_slice(buffer, 0, length);

        auto data = slice.data();

        return std::ranges::equal(data, std::views::single(TestType(message, length)));
    }));

    STATIC_REQUIRE(
        validate_predicate_over_nonempty_buffer([](const TestType buffer, auto) noexcept {
            auto slice = sendosio::buffer_slice(buffer, 0, 0);

            auto data = slice.data();

            return std::ranges::empty(data);
        }));
}

TEMPLATE_TEST_CASE(
    "buffer_slice(buffer, offset, length) can pick out an arbitrary subsequence",
    "[sendosio::buffer_slice]",
    sendosio::const_buffer,
    sendosio::mutable_buffer) {
    STATIC_REQUIRE(validate_predicate_over_nonempty_buffer([](const TestType buffer,
                                                              char* message) noexcept {
        // pick out the ','
        constexpr std::size_t offset = 5;
        constexpr std::size_t length = 1;

        BEMAN_SENDOSIO_CONTRACT_ASSERT(message[offset] == ',');

        auto slice = sendosio::buffer_slice(buffer, offset, length);

        auto data = slice.data();

        return std::ranges::equal(data,
                                  std::views::single(TestType(message + offset, length)));
    }));
}

TEMPLATE_TEST_CASE("buffer_slice(empty_range<TestType>(), ...) works",
                   "[sendosio::buffer_slice]",
                   sendosio::const_buffer,
                   sendosio::mutable_buffer) {
    constexpr auto predicate = [](auto... args) noexcept {
        auto range = std::ranges::empty_view<TestType>();

        auto slice = sendosio::buffer_slice(range, args...);

        auto data = slice.data();

        return std::ranges::empty(data);
    };

    STATIC_REQUIRE(predicate());
    STATIC_REQUIRE(predicate(5));
    STATIC_REQUIRE(predicate(5, 0));
    STATIC_REQUIRE(predicate((std::numeric_limits<std::size_t>::max)()));
    STATIC_REQUIRE(predicate(5, 10));
}

TEMPLATE_TEST_CASE("buffer_slice(array{buffer, buffer, ...}) duplicates buffer",
                   "[sendosio:buffer_slice]",
                   sendosio::const_buffer,
                   sendosio::mutable_buffer) {
    STATIC_REQUIRE(
        validate_predicate_over_nonempty_buffer([](const TestType buffer, auto) noexcept {
            std::array buffers{buffer, buffer};

            auto slice = sendosio::buffer_slice(buffers);

            auto data = slice.data();

            return std::ranges::equal(buffers, data);
        }));

    STATIC_REQUIRE(
        validate_predicate_over_nonempty_buffer([](const TestType buffer, auto) noexcept {
            std::array buffers{buffer, buffer, buffer, buffer, buffer};

            auto slice = sendosio::buffer_slice(buffers);

            auto data = slice.data();

            return std::ranges::equal(buffers, data);
        }));
}

TEMPLATE_TEST_CASE(
    "buffer_slice(std::array{buffer, buffer, ...}, prefix, length) slices correctly",
    "[sendosio::buffer_slice]",
    sendosio::const_buffer,
    sendosio::mutable_buffer) {
    STATIC_REQUIRE(
        validate_predicate_over_nonempty_buffer([](const TestType buffer, auto) noexcept {
            std::array buffers{buffer, buffer};

            const auto about_half = buffer.size() / 2;

            // create a slice that straddles the two copies
            auto slice = sendosio::buffer_slice(buffers, about_half, buffer.size());

            auto data = slice.data();

            std::array<TestType, 2> expected{
                // trim the first from the front
                buffer + about_half,
                // trim the second from the back
                sendosio::make_buffer(buffer, buffer.size() - about_half)};

            return std::ranges::equal(data, expected);
        }));

    STATIC_REQUIRE(
        validate_predicate_over_nonempty_buffer([](const TestType buffer, auto) noexcept {
            std::array buffers{buffer, buffer, buffer};

            const auto about_half = buffer.size() / 2;

            // create a slice that straddles the middle copy
            auto slice = sendosio::buffer_slice(buffers, about_half, 2 * buffer.size());

            auto data = slice.data();

            std::array<TestType, 3> expected{
                // trim the first from the front
                buffer + about_half,
                // include the whole middle
                buffer,
                // trim the second from the back
                sendosio::make_buffer(buffer, buffer.size() - about_half)};

            return std::ranges::equal(data, expected);
        }));

    STATIC_REQUIRE(
        validate_predicate_over_nonempty_buffer([](const TestType buffer, auto) noexcept {
            std::array buffers{buffer, buffer};

            auto slice = sendosio::buffer_slice(buffers, buffer.size() * 2);

            auto data = slice.data();

            return std::ranges::empty(data);
        }));
}

TEMPLATE_TEST_CASE("buffer_slice(...).remove_prefix(prefix) handles all contingencies",
                   "[sendosio::buffer_slice]",
                   sendosio::const_buffer,
                   sendosio::mutable_buffer) {

    STATIC_REQUIRE(
        validate_predicate_over_nonempty_buffer([](const TestType buffer, auto) noexcept {
            const auto about_half = buffer.size() / 2;

            auto slice = sendosio::buffer_slice(buffer);

            slice.remove_prefix(about_half);

            auto data = slice.data();

            return std::ranges::equal(data, std::views::single(buffer + about_half));
        }));

    STATIC_REQUIRE(
        validate_predicate_over_nonempty_buffer([](const TestType buffer, auto) noexcept {
            auto slice = sendosio::buffer_slice(buffer, 0, 2);

            slice.remove_prefix(1);

            auto data = slice.data();

            auto expected = sendosio::make_buffer(buffer, 2) + 1;

            return std::ranges::equal(data, std::views::single(expected));
        }));

    STATIC_REQUIRE(
        validate_predicate_over_nonempty_buffer([](const TestType buffer, auto) noexcept {
            auto slice = sendosio::buffer_slice(buffer, 0, 2);

            slice.remove_prefix(3);

            auto data = slice.data();

            return std::ranges::empty(data);
        }));
}

} // namespace
