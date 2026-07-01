// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "beman/sendosio/detail/buffers.hpp"

#include <catch2/catch_all.hpp>

#include <array>
#include <concepts>

namespace {

namespace sendosio = beman::sendosio;

TEST_CASE(
    "(an array of) const_buffer and mutable_buffer both satisify const_buffer_sequence",
    "[sendosio::buffers]") {
    STATIC_REQUIRE(sendosio::const_buffer_sequence<sendosio::const_buffer>);
    STATIC_REQUIRE(sendosio::const_buffer_sequence<sendosio::mutable_buffer>);

    STATIC_REQUIRE(
        sendosio::const_buffer_sequence<std::array<sendosio::const_buffer, 10>>);
    STATIC_REQUIRE(
        sendosio::const_buffer_sequence<std::array<sendosio::mutable_buffer, 10>>);
}

TEST_CASE(
    "(an array of) mutable_buffer but not const_buffer satisfies mutable_buffer_sequence",
    "[sendosio::buffers]") {
    STATIC_REQUIRE(!sendosio::mutable_buffer_sequence<sendosio::const_buffer>);
    STATIC_REQUIRE(sendosio::mutable_buffer_sequence<sendosio::mutable_buffer>);

    STATIC_REQUIRE(
        !sendosio::mutable_buffer_sequence<std::array<sendosio::const_buffer, 10>>);
    STATIC_REQUIRE(
        sendosio::mutable_buffer_sequence<std::array<sendosio::mutable_buffer, 10>>);
}

template <class TestType, std::invocable<TestType&> Predicate>
    requires std::same_as<bool, std::invoke_result_t<Predicate, TestType&>>
consteval bool validate_constexpr_terminal_iterators(Predicate pred) noexcept {
    TestType buffer{};

    return pred(buffer);
}

TEMPLATE_TEST_CASE("begin and end work on const and mutable buffer instances",
                   "[sendosio::buffers]",
                   const sendosio::const_buffer,
                   sendosio::const_buffer,
                   const sendosio::mutable_buffer,
                   sendosio::mutable_buffer) {
    STATIC_REQUIRE(validate_constexpr_terminal_iterators<TestType>(
        [](TestType& buffer) noexcept { return &*sendosio::begin(buffer) == &buffer; }));

    STATIC_REQUIRE(
        validate_constexpr_terminal_iterators<TestType>([](TestType& buffer) noexcept {
            return &*sendosio::end(buffer) == (&buffer + 1);
        }));
}

TEMPLATE_TEST_CASE("begin and end work on const and mutable buffer sequences",
                   "[sendosio::buffers]",
                   (const std::array<sendosio::const_buffer, 5>),
                   (std::array<sendosio::const_buffer, 5>),
                   (const std::array<sendosio::mutable_buffer, 5>),
                   (std::array<sendosio::mutable_buffer, 5>)) {
    STATIC_REQUIRE(
        validate_constexpr_terminal_iterators<TestType>([](TestType& buffers) noexcept {
            return sendosio::begin(buffers) == std::ranges::begin(buffers);
        }));

    STATIC_REQUIRE(
        validate_constexpr_terminal_iterators<TestType>([](TestType& buffers) noexcept {
            return sendosio::end(buffers) == std::ranges::end(buffers);
        }));
}

template <class TestType>
consteval std::size_t size_of_empty_constexpr_buffer() noexcept {
    TestType buffer{};
    return sendosio::buffer_size(buffer);
}

template <class TestType>
consteval std::size_t size_of_nonempty_constexpr_buffer() noexcept {
    char     message[] = "hello, world!";
    TestType buffer(message, sizeof(message));

    return sendosio::buffer_size(buffer);
}

TEMPLATE_TEST_CASE("buffer_size works on const_buffer and mutable_buffer",
                   "[sendosio::buffers]",
                   const sendosio::const_buffer,
                   sendosio::const_buffer,
                   const sendosio::mutable_buffer,
                   sendosio::mutable_buffer) {
    STATIC_REQUIRE(size_of_empty_constexpr_buffer<TestType>() == 0);

    STATIC_REQUIRE(size_of_nonempty_constexpr_buffer<TestType>() == 14);
}

template <class TestType>
consteval std::size_t size_of_nonempty_constexpr_buffer_sequence() noexcept {
    char message1[] = "hello, world!";
    char message2[] = "goodbye, world!";

    using buffer_t = TestType::value_type;

    TestType buffer{buffer_t(message1, sizeof(message1)),
                    buffer_t(message2, sizeof(message2))};

    return sendosio::buffer_size(buffer);
}

TEMPLATE_TEST_CASE("buffer_size works on const_buffer and mutable_buffer sequences",
                   "[sendosio::buffers]",
                   (const std::array<sendosio::const_buffer, 2>),
                   (std::array<sendosio::const_buffer, 2>),
                   (const std::array<sendosio::mutable_buffer, 2>),
                   (std::array<sendosio::mutable_buffer, 2>)) {
    STATIC_REQUIRE(size_of_empty_constexpr_buffer<TestType>() == 0);

    STATIC_REQUIRE(size_of_nonempty_constexpr_buffer_sequence<TestType>() == 30);
}

template <class TestType>
consteval bool empty_constexpr_buffer_is_empty() noexcept {
    TestType empty{};
    return sendosio::buffer_empty(empty);
}

template <class TestType>
consteval bool nonempty_constexpr_buffer_is_empty() noexcept {
    char     message[] = "hello, world!";
    TestType buffer(message, sizeof(message));

    return sendosio::buffer_empty(buffer);
}

TEMPLATE_TEST_CASE("buffer_empty works on const_buffer and mutable_buffer",
                   "[sendosio::buffers]",
                   const sendosio::const_buffer,
                   sendosio::const_buffer,
                   const sendosio::mutable_buffer,
                   sendosio::mutable_buffer) {
    STATIC_REQUIRE(empty_constexpr_buffer_is_empty<TestType>());

    STATIC_REQUIRE(!nonempty_constexpr_buffer_is_empty<TestType>());
}

template <class TestType>
consteval bool nonempty_constexpr_buffer_sequence_is_empty() noexcept {
    char message1[] = "hello, world!";
    char message2[] = "goodbye, world!";

    using buffer_t = TestType::value_type;

    TestType buffer{buffer_t(message1, sizeof(message1)),
                    buffer_t(message2, sizeof(message2))};

    return sendosio::buffer_empty(buffer);
}

TEMPLATE_TEST_CASE("buffer_empty works on const_buffer and mutable_buffer sequences",
                   "[sendosio::buffers]",
                   (const std::array<sendosio::const_buffer, 2>),
                   (std::array<sendosio::const_buffer, 2>),
                   (const std::array<sendosio::mutable_buffer, 2>),
                   (std::array<sendosio::mutable_buffer, 2>)) {
    STATIC_REQUIRE(empty_constexpr_buffer_is_empty<TestType>());

    STATIC_REQUIRE(!nonempty_constexpr_buffer_sequence_is_empty<TestType>());
}

template <class TestType>
consteval std::size_t length_of_empty_constexpr_buffer() noexcept {
    TestType empty;

    return sendosio::buffer_length(empty);
}

template <class TestType>
consteval std::size_t length_of_nonempty_constexpr_buffer() noexcept {
    char     message[] = "hello, world!";
    TestType buffer(message, sizeof(message));

    return sendosio::buffer_length(buffer);
}

TEMPLATE_TEST_CASE("buffer_length works on const_buffer and mutable_buffer",
                   "[sendosio::buffers]",
                   const sendosio::const_buffer,
                   sendosio::const_buffer,
                   const sendosio::mutable_buffer,
                   sendosio::mutable_buffer) {
    STATIC_REQUIRE(length_of_empty_constexpr_buffer<TestType>() == 1);

    STATIC_REQUIRE(length_of_nonempty_constexpr_buffer<TestType>() == 1);
}

template <class TestType>
consteval std::size_t length_of_nonempty_constexpr_buffer_sequence() noexcept {
    using buffer_t = TestType::value_type;

    char message1[] = "hello, world!";
    char message2[] = "goodbye, world!";

    TestType buffer{buffer_t(message1, sizeof(message1)),
                    buffer_t(message2, sizeof(message2))};

    return sendosio::buffer_length(buffer);
}

TEMPLATE_TEST_CASE("buffer_length works on const_buffer and mutable_buffer sequences",
                   "[sendosio::buffers]",
                   (const std::array<sendosio::const_buffer, 2>),
                   (std::array<sendosio::const_buffer, 2>),
                   (const std::array<sendosio::mutable_buffer, 2>),
                   (std::array<sendosio::mutable_buffer, 2>)) {
    STATIC_REQUIRE(length_of_empty_constexpr_buffer<TestType>() == 2);

    STATIC_REQUIRE(length_of_nonempty_constexpr_buffer_sequence<TestType>() == 2);
}

} // namespace
