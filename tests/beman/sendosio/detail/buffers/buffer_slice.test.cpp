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
    STATIC_REQUIRE(validate_predicate_over_nonempty_buffer([](TestType buffer,
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
        [](TestType buffer, char* message) noexcept {
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
        validate_predicate_over_nonempty_buffer([](TestType buffer, auto) noexcept {
            // remove exactly the whole buffer
            auto slice = sendosio::buffer_slice(buffer, buffer.size());

            auto data = slice.data();

            // there's no data left in the slice so the range is empty
            return std::ranges::empty(data);
        }));

    STATIC_REQUIRE(
        validate_predicate_over_nonempty_buffer([](TestType buffer, auto) noexcept {
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
    STATIC_REQUIRE(validate_predicate_over_nonempty_buffer([](TestType buffer,
                                                              char*    message) noexcept {
        constexpr std::size_t length = 5; // limit to "hello"

        BEMAN_SENDOSIO_CONTRACT_ASSERT(buffer.size() > length);

        auto slice = sendosio::buffer_slice(buffer, 0, length);

        auto data = slice.data();

        return std::ranges::equal(data, std::views::single(TestType(message, length)));
    }));

    STATIC_REQUIRE(
        validate_predicate_over_nonempty_buffer([](TestType buffer, auto) noexcept {
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
    STATIC_REQUIRE(validate_predicate_over_nonempty_buffer([](TestType buffer,
                                                              char*    message) noexcept {
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
    STATIC_REQUIRE(validate_predicate_over_nonempty_buffer(
        [](TestType buffer, char* message) noexcept {
            std::array buffers{buffer, buffer};

            auto slice = sendosio::buffer_slice(buffers);

            auto data = slice.data();

            return std::ranges::equal(buffers, data);
        }));

    STATIC_REQUIRE(validate_predicate_over_nonempty_buffer(
        [](TestType buffer, char* message) noexcept {
            std::array buffers{buffer, buffer, buffer, buffer, buffer};

            auto slice = sendosio::buffer_slice(buffers);

            auto data = slice.data();

            return std::ranges::equal(buffers, data);
        }));
}

template <std::size_t ExtraCount>
constexpr auto // std::array<std::array<char, ExtraSize>, ExtraCount>
make_extra_data() {
    char              message[] = "hello, extra data!";
    const std::size_t size      = sizeof(message);

    std::array<std::array<char, size>, ExtraCount> ret{};

    for (auto& arr : ret) {
        std::ranges::copy(message, arr.data());
    }

    return ret;
}

TEMPLATE_TEST_CASE(
    "buffer_slice(ranges::join(...)) produces a slice over several buffers",
    "[sendosio::buffer_slice]",
    sendosio::const_buffer,
    sendosio::mutable_buffer) {
    STATIC_REQUIRE(
        validate_predicate_over_nonempty_buffer([](TestType buffer, auto) noexcept {
            constexpr std::size_t extra_count = 3;

            auto extra_data = make_extra_data<extra_count>();

            std::array<TestType, extra_count + 1> buffers{
                buffer,
                TestType(sendosio::make_buffer(extra_data[0])),
                TestType(sendosio::make_buffer(extra_data[1])),
                TestType(sendosio::make_buffer(extra_data[2]))};

            auto slice = sendosio::buffer_slice(buffers);

            auto data = slice.data();

            return std::ranges::equal(data, buffers);
        }));
}

} // namespace
