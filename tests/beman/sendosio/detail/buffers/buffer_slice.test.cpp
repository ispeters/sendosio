// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "beman/sendosio/detail/buffers/buffer_slice.hpp"

#include <catch2/catch_all.hpp>

#include <beman/sendosio/detail/buffers.hpp>
#include <beman/sendosio/detail/buffers/make_buffer.hpp>
#include <beman/sendosio/detail/contracts.hpp>

#include <array>
#include <ranges>

namespace {

// TODO: this file probably needs more test cases; for one thing, it would be a good idea
//       to confirm that the results of `buffer_slice(buffers)` are independent views into
//       `buffers`

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
    STATIC_REQUIRE(
        validate_predicate_over_nonempty_buffer([](const TestType buffer, auto) noexcept {
            auto slice = sendosio::buffer_slice(buffer);

            static_assert(
                std::same_as<TestType, std::ranges::range_value_t<decltype(slice)>>);

            // this asserts that data contains one buffer that is equal to buffer, i.e.
            // that buffer_slice(buffer) is the identity
            return std::ranges::equal(slice, std::views::single(buffer));
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

            // slice points into buffer so, to validate that slice is correct, we
            // can't touch buffer; instead, make a copy, mutate it, and compare to the
            // result
            auto buffer_copy = buffer;
            buffer_copy += offset;

            BEMAN_SENDOSIO_CONTRACT_ASSERT(buffer_copy.data() == (message + offset));

            return std::ranges::equal(slice, std::views::single(buffer_copy));
        }));

    STATIC_REQUIRE(
        validate_predicate_over_nonempty_buffer([](const TestType buffer, auto) noexcept {
            // remove exactly the whole buffer
            auto slice = sendosio::buffer_slice(buffer, buffer.size());

            // there's no data left in the slice so the range is empty
            return std::ranges::empty(slice);
        }));

    STATIC_REQUIRE(
        validate_predicate_over_nonempty_buffer([](const TestType buffer, auto) noexcept {
            // remove the most it is possible to remove
            auto slice =
                sendosio::buffer_slice(buffer, (std::numeric_limits<std::size_t>::max)());

            // the result of offsetting "into infinity" is still just an empty range
            return std::ranges::empty(slice);
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

        return std::ranges::equal(slice, std::views::single(TestType(message, length)));
    }));

    STATIC_REQUIRE(
        validate_predicate_over_nonempty_buffer([](const TestType buffer, auto) noexcept {
            auto slice = sendosio::buffer_slice(buffer, 0, 0);

            return std::ranges::empty(slice);
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

        return std::ranges::equal(slice,
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

        return std::ranges::empty(slice);
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

            return std::ranges::equal(buffers, slice);
        }));

    STATIC_REQUIRE(
        validate_predicate_over_nonempty_buffer([](const TestType buffer, auto) noexcept {
            std::array buffers{buffer, buffer, buffer, buffer, buffer};

            auto slice = sendosio::buffer_slice(buffers);

            return std::ranges::equal(buffers, slice);
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

            std::array<TestType, 2> expected{
                // trim the first from the front
                buffer + about_half,
                // trim the second from the back
                sendosio::make_buffer(buffer, buffer.size() - about_half)};

            return std::ranges::equal(slice, expected);
        }));

    STATIC_REQUIRE(
        validate_predicate_over_nonempty_buffer([](const TestType buffer, auto) noexcept {
            std::array buffers{buffer, buffer, buffer};

            const auto about_half = buffer.size() / 2;

            // create a slice that straddles the middle copy
            auto slice = sendosio::buffer_slice(buffers, about_half, 2 * buffer.size());

            std::array<TestType, 3> expected{
                // trim the first from the front
                buffer + about_half,
                // include the whole middle
                buffer,
                // trim the second from the back
                sendosio::make_buffer(buffer, buffer.size() - about_half)};

            return std::ranges::equal(slice, expected);
        }));

    STATIC_REQUIRE(
        validate_predicate_over_nonempty_buffer([](const TestType buffer, auto) noexcept {
            std::array buffers{buffer, buffer};

            auto slice = sendosio::buffer_slice(buffers, buffer.size() * 2);

            return std::ranges::empty(slice);
        }));
}

TEMPLATE_TEST_CASE("buffer_slice accepts slice_of<It, true> instances",
                   "[sendosio::buffer_slice]",
                   sendosio::const_buffer,
                   sendosio::mutable_buffer) {
    char     message[] = "hello, world!";
    TestType buffer    = sendosio::make_buffer(message);

    auto source_slice = sendosio::buffer_slice(buffer);

    REQUIRE(*std::ranges::begin(source_slice) == buffer);

    auto dest_slice = sendosio::buffer_slice(source_slice);

    REQUIRE(*std::ranges::begin(dest_slice) == buffer);
}

TEMPLATE_TEST_CASE("buffer_slice accepts slice_of<It, false> instances",
                   "[sendosio::buffer_slice]",
                   sendosio::const_buffer,
                   sendosio::mutable_buffer) {
    char     message[] = "hello, world!";
    TestType buffer    = sendosio::make_buffer(message);

    auto reversed = std::views::reverse(sendosio::buffer_slice(buffer));

    REQUIRE(*std::ranges::begin(reversed) == buffer);

    auto source_slice = sendosio::buffer_slice(reversed);

    using slice_t = decltype(source_slice);

    STATIC_REQUIRE(
        std::same_as<
            slice_t,
            sendosio::slice_of_detail::slice_of<typename slice_t::iterator_type, false>>);

    REQUIRE(*std::ranges::begin(source_slice) == buffer);

    auto dest_slice = sendosio::buffer_slice(source_slice);

    STATIC_REQUIRE(std::same_as<slice_t, decltype(dest_slice)>);

    REQUIRE(*std::ranges::begin(dest_slice) == buffer);
}

} // namespace
