// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "beman/sendosio/detail/buffers.hpp"

#include <catch2/catch_all.hpp>

#include <array>
#include <concepts>
#include <limits>

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
consteval bool validate_predicate_over_empty_constexpr_buffer(Predicate pred) noexcept {
    TestType buffer{};

    return pred(buffer);
}

TEMPLATE_TEST_CASE("begin and end work on const and mutable buffer instances",
                   "[sendosio::buffers]",
                   const sendosio::const_buffer,
                   sendosio::const_buffer,
                   const sendosio::mutable_buffer,
                   sendosio::mutable_buffer) {
    STATIC_REQUIRE(validate_predicate_over_empty_constexpr_buffer<TestType>(
        [](TestType& buffer) noexcept { return &*sendosio::begin(buffer) == &buffer; }));

    STATIC_REQUIRE(validate_predicate_over_empty_constexpr_buffer<TestType>(
        [](TestType& buffer) noexcept {
            return &*sendosio::end(buffer) == (&buffer + 1);
        }));
}

TEMPLATE_TEST_CASE("begin and end work on const and mutable buffer sequences",
                   "[sendosio::buffers]",
                   (const std::array<sendosio::const_buffer, 5>),
                   (std::array<sendosio::const_buffer, 5>),
                   (const std::array<sendosio::mutable_buffer, 5>),
                   (std::array<sendosio::mutable_buffer, 5>)) {
    STATIC_REQUIRE(validate_predicate_over_empty_constexpr_buffer<TestType>(
        [](TestType& buffers) noexcept {
            return sendosio::begin(buffers) == std::ranges::begin(buffers);
        }));

    STATIC_REQUIRE(validate_predicate_over_empty_constexpr_buffer<TestType>(
        [](TestType& buffers) noexcept {
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

TEMPLATE_TEST_CASE("buffer types are nothrow default-constructible",
                   "[sendosio::buffers]",
                   sendosio::const_buffer,
                   sendosio::mutable_buffer) {
    STATIC_REQUIRE(std::is_nothrow_default_constructible_v<TestType>);
}

TEMPLATE_TEST_CASE("buffer types are nothrow constructible from (pointer, size)",
                   "[sendosio::buffers]",
                   sendosio::const_buffer,
                   sendosio::mutable_buffer) {
    STATIC_REQUIRE(std::is_nothrow_constructible_v<TestType, void*, std::size_t>);
    STATIC_REQUIRE(std::is_nothrow_constructible_v<TestType, char*, std::size_t>);
    STATIC_REQUIRE(
        std::is_nothrow_constructible_v<TestType, unsigned char*, std::size_t>);
    STATIC_REQUIRE(std::is_nothrow_constructible_v<TestType, signed char*, std::size_t>);
    STATIC_REQUIRE(std::is_nothrow_constructible_v<TestType, std::byte*, std::size_t>);
}

TEST_CASE("const_buffer is nothrow constructible from (const pointer, size)",
          "[sendosio::buffers]") {
    STATIC_REQUIRE(std::is_nothrow_constructible_v<sendosio::const_buffer,
                                                   const void*,
                                                   std::size_t>);
    STATIC_REQUIRE(std::is_nothrow_constructible_v<sendosio::const_buffer,
                                                   const char*,
                                                   std::size_t>);
    STATIC_REQUIRE(std::is_nothrow_constructible_v<sendosio::const_buffer,
                                                   const unsigned char*,
                                                   std::size_t>);
    STATIC_REQUIRE(std::is_nothrow_constructible_v<sendosio::const_buffer,
                                                   const signed char*,
                                                   std::size_t>);
    STATIC_REQUIRE(std::is_nothrow_constructible_v<sendosio::const_buffer,
                                                   const std::byte*,
                                                   std::size_t>);
}

TEST_CASE("mutable_buffer is not constructible from (const pointer, size)",
          "[sendosio::buffers]") {
    STATIC_REQUIRE(
        !std::is_constructible_v<sendosio::mutable_buffer, const void*, std::size_t>);
    STATIC_REQUIRE(
        !std::is_constructible_v<sendosio::mutable_buffer, const char*, std::size_t>);
    STATIC_REQUIRE(!std::is_constructible_v<sendosio::mutable_buffer,
                                            const unsigned char*,
                                            std::size_t>);
    STATIC_REQUIRE(!std::is_constructible_v<sendosio::mutable_buffer,
                                            const signed char*,
                                            std::size_t>);
    STATIC_REQUIRE(!std::is_constructible_v<sendosio::mutable_buffer,
                                            const std::byte*,
                                            std::size_t>);
}

TEMPLATE_TEST_CASE("buffer types are not constructible from explicit (nullptr, size)",
                   "[sendosio::buffers]",
                   sendosio::const_buffer,
                   sendosio::mutable_buffer) {
    STATIC_REQUIRE(!std::constructible_from<TestType, std::nullptr_t, std::size_t>);
}

TEMPLATE_TEST_CASE("buffer types are trivially movable and copyable",
                   "[sendosio::buffers]",
                   sendosio::const_buffer,
                   sendosio::mutable_buffer) {
    STATIC_REQUIRE(std::is_nothrow_copy_constructible_v<TestType>);
    STATIC_REQUIRE(std::is_trivially_copy_constructible_v<TestType>);

    STATIC_REQUIRE(std::is_nothrow_move_constructible_v<TestType>);
    STATIC_REQUIRE(std::is_trivially_move_constructible_v<TestType>);

    STATIC_REQUIRE(std::is_nothrow_copy_assignable_v<TestType>);
    STATIC_REQUIRE(std::is_trivially_copy_assignable_v<TestType>);

    STATIC_REQUIRE(std::is_nothrow_move_assignable_v<TestType>);
    STATIC_REQUIRE(std::is_trivially_move_assignable_v<TestType>);
}

TEST_CASE("buffer types are regular and mutually equality comparable",
          "[sendosio::buffers]") {
    STATIC_REQUIRE(std::regular<sendosio::mutable_buffer>);
    STATIC_REQUIRE(std::regular<sendosio::const_buffer>);

    STATIC_REQUIRE(
        std::equality_comparable_with<sendosio::mutable_buffer, sendosio::const_buffer>);
    STATIC_REQUIRE(
        std::equality_comparable_with<sendosio::const_buffer, sendosio::mutable_buffer>);
}

template <class TestType, std::invocable<TestType&, char*, std::size_t> Predicate>
    requires std::same_as<bool,
                          std::invoke_result_t<Predicate, TestType&, char*, std::size_t>>
consteval bool
validate_predicate_over_nonempty_constexpr_buffer(Predicate pred) noexcept {
    char message[] = "hello, world!";

    TestType buffer(message, sizeof(message));

    return pred(buffer, static_cast<char*>(message), sizeof(message));
}

TEMPLATE_TEST_CASE("buffer.data() returns the address of the memory it is a view of",
                   "[sendosio::buffers]",
                   const sendosio::const_buffer,
                   sendosio::const_buffer,
                   const sendosio::mutable_buffer,
                   sendosio::mutable_buffer) {
    STATIC_REQUIRE(validate_predicate_over_empty_constexpr_buffer<TestType>(
        [](TestType& buffer) noexcept { return buffer.data() == nullptr; }));

    STATIC_REQUIRE(validate_predicate_over_nonempty_constexpr_buffer<TestType>(
        [](TestType& buffer, const char* message, auto) noexcept {
            return buffer.data() == message;
        }));
}

TEMPLATE_TEST_CASE("buffer.size() returns the size of the memory it is a view of",
                   "[sendosio::buffers]",
                   const sendosio::const_buffer,
                   sendosio::const_buffer,
                   const sendosio::mutable_buffer,
                   sendosio::mutable_buffer) {
    STATIC_REQUIRE(validate_predicate_over_empty_constexpr_buffer<TestType>(
        [](TestType& buffer) noexcept { return buffer.size() == 0; }));

    STATIC_REQUIRE(validate_predicate_over_nonempty_constexpr_buffer<TestType>(
        [](TestType& buffer, const char*, std::size_t size) noexcept {
            return buffer.size() == size;
        }));
}

TEMPLATE_TEST_CASE("operator+= trims from the front and clamps",
                   "[sendosio::buffers]",
                   sendosio::const_buffer,
                   sendosio::mutable_buffer) {
    STATIC_REQUIRE(validate_predicate_over_empty_constexpr_buffer<TestType>(
        [](TestType& buffer) noexcept {
            buffer += 5;
            return (buffer.data() == nullptr) && (buffer.size() == 0);
        }));

    STATIC_REQUIRE(validate_predicate_over_nonempty_constexpr_buffer<TestType>(
        [](TestType& buffer, const char* message, std::size_t size) noexcept {
            // choose an increment that cuts the buffer approximately in half and make
            // sure the increment is at least 1 so this test isn't a no-op
            const std::size_t increment = (std::max)(size / 2, std::size_t(1));

            buffer += increment;

            return (buffer.size() == (size - increment)) &&
                   (buffer.data() == (message + increment));
        }));

    STATIC_REQUIRE(validate_predicate_over_nonempty_constexpr_buffer<TestType>(
        [](TestType& buffer, const char* message, std::size_t size) noexcept {
            // increment the buffer by such a gigantic amount that it must clamp; also
            // validate that we handle extreme values
            buffer += (std::numeric_limits<std::size_t>::max)();

            return (buffer.size() == 0) && (buffer.data() == (message + size));
        }));
}

} // namespace
