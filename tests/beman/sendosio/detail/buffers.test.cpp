// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/sendosio/config.hpp>

#include <catch2/catch_all.hpp>

#include <beman/sendosio/detail/buffers.hpp>

#if BEMAN_SENDOSIO_USE_MODULES()
import std;
#else
    #include <concepts>
    #include <limits>
    #include <type_traits>
#endif

namespace {

namespace sendosio = beman::sendosio;

template <class TestType, std::invocable<TestType&> Predicate>
    requires std::same_as<bool, std::invoke_result_t<Predicate, TestType&>>
constexpr bool validate_predicate_over_empty_constexpr_buffer(Predicate pred) noexcept {
    TestType buffer{};

    return pred(buffer);
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
constexpr bool
validate_predicate_over_nonempty_constexpr_buffer(Predicate pred) noexcept {
    char message[] = "hello, world!";

    TestType buffer(message, sizeof(message));

    return pred(buffer, static_cast<char*>(message), sizeof(message));
}

TEMPLATE_TEST_CASE("buffer(void*, size) works as expected",
                   "[sendosio::buffers]",
                   sendosio::const_buffer,
                   sendosio::mutable_buffer) {
    REQUIRE(validate_predicate_over_empty_constexpr_buffer<TestType>(
        [](TestType buffer) noexcept {
            TestType withVoid(static_cast<void*>(nullptr), 0);
            return withVoid == buffer;
        }));

    REQUIRE(validate_predicate_over_nonempty_constexpr_buffer<TestType>(
        [](TestType buffer, void* data, std::size_t size) noexcept {
            TestType withVoid(data, size);
            return withVoid == buffer;
        }));
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
