// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "beman/sendosio/detail/buffers/make_buffer.hpp"

#include <catch2/catch_all.hpp>

#include <concepts>

namespace {

namespace sendosio = beman::sendosio;

TEMPLATE_TEST_CASE("make_buffer identity has the expected return type",
                   "[sendosio::make_buffer]",
                   sendosio::mutable_buffer,
                   sendosio::mutable_buffer&,
                   const sendosio::mutable_buffer&,
                   sendosio::const_buffer,
                   sendosio::const_buffer&,
                   const sendosio::const_buffer&) {
    STATIC_REQUIRE(std::same_as<std::remove_cvref_t<TestType>,
                                std::invoke_result_t<sendosio::make_buffer_t, TestType>>);
}

TEMPLATE_TEST_CASE("make_buffer identity works with empty buffers",
                   "[sendosio::make_buffer]",
                   sendosio::mutable_buffer,
                   sendosio::const_buffer) {
    STATIC_REQUIRE(sendosio::make_buffer(TestType()) == TestType());
}

template <std::invocable<char*, std::size_t> Predicate>
    requires std::same_as<bool, std::invoke_result_t<Predicate, char*, std::size_t>>
[[nodiscard]] constexpr bool
validate_predicate_over_nonempty_message(Predicate pred) noexcept {
    char message[] = "hello, world!";

    return pred(static_cast<char*>(message), sizeof(message));
}

TEMPLATE_TEST_CASE("make_buffer identity works with non-empty buffers",
                   "[sendosio::make_buffer]",
                   sendosio::mutable_buffer,
                   sendosio::const_buffer) {
    STATIC_REQUIRE(validate_predicate_over_nonempty_message(
        [](char* data, std::size_t size) noexcept {
            TestType buffer(data, size);
            return buffer == sendosio::make_buffer(buffer);
        }));
}

TEMPLATE_TEST_CASE("make_buffer(pointer, size) works",
                   "[sendosio::make_buffer]",
                   char,
                   const char) {
    STATIC_REQUIRE(validate_predicate_over_nonempty_message(
        [](TestType* data, std::size_t size) noexcept {
            auto buffer = sendosio::make_buffer(data, size);
            return buffer == sendosio::const_buffer(data, size);
        }));

    using void_pointer = std::conditional_t<std::is_const_v<TestType>, const void, void>*;

    TestType message[] = "hello, world!";

    void_pointer pointer = message;

    auto buffer = sendosio::make_buffer(pointer, sizeof(message));

    REQUIRE(buffer.data() == message);
    REQUIRE(buffer.size() == sizeof(message));

    if constexpr (std::is_const_v<TestType>) {
        STATIC_REQUIRE(std::same_as<sendosio::const_buffer, decltype(buffer)>);
    } else {
        STATIC_REQUIRE(std::same_as<sendosio::mutable_buffer, decltype(buffer)>);
    }
}

TEMPLATE_TEST_CASE("make_buffer(pointer, size, max_size) works",
                   "[sendosio::make_buffer]",
                   char,
                   const char) {
    STATIC_REQUIRE(validate_predicate_over_nonempty_message(
        [](TestType* data, std::size_t size) noexcept {
            const std::size_t max_size = (std::max)(size / 2, std::size_t(1));

            auto buffer = sendosio::make_buffer(data, size, max_size);

            return buffer == sendosio::const_buffer(data, max_size);
        }));
}

} // namespace
