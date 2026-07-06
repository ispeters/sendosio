// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "beman/sendosio/detail/buffers/buffer_slice.hpp"

#include <catch2/catch_all.hpp>

#include <beman/sendosio/detail/buffers.hpp>
#include <beman/sendosio/detail/buffers/make_buffer.hpp>
#include <beman/sendosio/detail/contracts.hpp>

#include <array>

namespace {

namespace sendosio = beman::sendosio;

template <class TestType, std::invocable<TestType, char*> Predicate>
    requires std::same_as<bool, std::invoke_result_t<Predicate&, TestType, char*>>
constexpr bool validate_predicate_over_nonempty_buffer(Predicate pred) noexcept {
    char message[] = "hello, world!";
    return pred(TestType(sendosio::make_buffer(message)), static_cast<char*>(message));
}

TEMPLATE_TEST_CASE("buffer_slice(buffer) is the identity",
                   "[sendosio::buffer_slice]",
                   sendosio::const_buffer,
                   sendosio::mutable_buffer) {
    STATIC_REQUIRE(validate_predicate_over_nonempty_buffer<TestType>([](TestType buffer,
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
    STATIC_REQUIRE(validate_predicate_over_nonempty_buffer<TestType>(
        [](TestType buffer, char* message) noexcept {
            constexpr std::size_t offset = 7; // skip past "hello, "

            auto slice = sendosio::buffer_slice(buffer, offset);

            auto data = slice.data();

            // slice and data point into buffer so, to validate that data is correct, we
            // can't touch buffer; instead, make a copy, mutate it, and compare to the
            // result
            auto buffer_copy = buffer;
            buffer_copy += offset;

            return buffer_copy.data() == (message + offset) &&
                   std::ranges::equal(data, std::views::single(buffer_copy));
        }));

    STATIC_REQUIRE(validate_predicate_over_nonempty_buffer<TestType>(
        [](TestType buffer, auto) noexcept {
            // remove exactly the whole buffer
            auto slice = sendosio::buffer_slice(buffer, buffer.size());

            auto data = slice.data();

            // there's no data left in the slice so the range is empty
            return std::ranges::empty(data);
        }));

    STATIC_REQUIRE(validate_predicate_over_nonempty_buffer<TestType>(
        [](TestType buffer, auto) noexcept {
            // remove the most it is possible to remove
            auto slice =
                sendosio::buffer_slice(buffer, (std::numeric_limits<std::size_t>::max)());

            auto data = slice.data();

            // the result of offsetting "into infinity" is still just an empty range
            return std::ranges::empty(data);
        }));
}

} // namespace
