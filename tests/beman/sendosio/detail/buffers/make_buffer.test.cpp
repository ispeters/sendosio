// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "beman/sendosio/detail/buffers/make_buffer.hpp"

#include <catch2/catch_all.hpp>

#include <beman/sendosio/detail/contracts.hpp>

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <string_view>
#include <type_traits>
#include <vector>

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

template <class CharT>
using void_pointer_t = std::conditional_t<std::is_const_v<CharT>, const void, void>*;

template <class CharT>
using buffer_type_t = std::conditional_t<std::is_const_v<CharT>,
                                         sendosio::const_buffer,
                                         sendosio::mutable_buffer>;

constexpr std::size_t at_least_half(std::size_t size) noexcept {
    return (std::max)(size / 2, std::size_t(1));
};

TEMPLATE_TEST_CASE("make_buffer(pointer, size) works",
                   "[sendosio::make_buffer]",
                   char,
                   const char) {
    STATIC_REQUIRE(validate_predicate_over_nonempty_message(
        [](TestType* data, std::size_t size) noexcept {
            auto buffer = sendosio::make_buffer(data, size);
            return buffer == sendosio::const_buffer(data, size);
        }));

    REQUIRE(validate_predicate_over_nonempty_message(
        [](void_pointer_t<TestType> data, std::size_t size) noexcept {
            auto buffer = sendosio::make_buffer(data, size);

            STATIC_REQUIRE(std::same_as<buffer_type_t<TestType>, decltype(buffer)>);

            return buffer == sendosio::const_buffer(data, size);
        }));
}

TEMPLATE_TEST_CASE("make_buffer(pointer, size, max_size) works",
                   "[sendosio::make_buffer]",
                   char,
                   const char) {
    STATIC_REQUIRE(validate_predicate_over_nonempty_message(
        [](TestType* data, std::size_t size) noexcept {
            const std::size_t max_size = at_least_half(size);

            auto buffer = sendosio::make_buffer(data, size, max_size);

            return buffer == sendosio::const_buffer(data, max_size);
        }));

    REQUIRE(validate_predicate_over_nonempty_message(
        [](void_pointer_t<TestType> data, std::size_t size) noexcept {
            const std::size_t max_size = at_least_half(size);

            auto buffer = sendosio::make_buffer(data, size, max_size);

            STATIC_REQUIRE(std::same_as<buffer_type_t<TestType>, decltype(buffer)>);

            return buffer == sendosio::const_buffer(data, max_size);
        }));
}

TEST_CASE("make_buffer(string_view) works at constexpr time", "[sendosio::make_buffer]") {
    STATIC_REQUIRE(validate_predicate_over_nonempty_message(
        [](char* data, std::size_t size) noexcept {
            std::string_view view(data, size);

            auto buffer = sendosio::make_buffer(view);

            BEMAN_SENDOSIO_CONTRACT_ASSERT(
                std::same_as<sendosio::const_buffer, decltype(buffer)>);

            return buffer.data() == data && buffer.size() == size;
        }));
}

TEST_CASE("make_buffer(string_view, max_size) works at constexpr time",
          "[sendosio::make_buffer]") {
    STATIC_REQUIRE(validate_predicate_over_nonempty_message(
        [](char* data, std::size_t size) noexcept {
            std::string_view view(data, size);

            const std::size_t max_size = at_least_half(size);

            auto buffer = sendosio::make_buffer(view, max_size);

            BEMAN_SENDOSIO_CONTRACT_ASSERT(
                std::same_as<sendosio::const_buffer, decltype(buffer)>);

            return buffer.data() == data && buffer.size() == max_size;
        }));

    STATIC_REQUIRE(validate_predicate_over_nonempty_message(
        [](char* data, std::size_t size) noexcept {
            std::string_view view(data, size);

            auto buffer = sendosio::make_buffer(view, 2 * size);

            BEMAN_SENDOSIO_CONTRACT_ASSERT(
                std::same_as<sendosio::const_buffer, decltype(buffer)>);

            return buffer.data() == data && buffer.size() == size;
        }));
}

TEMPLATE_TEST_CASE("make_buffer(Range&&) and make_buffer(Range&&, max_size) work",
                   "[sendosio::make_buffer]",
                   std::wstring_view,
                   const std::wstring_view,
                   std::u8string_view,
                   const std::u8string_view,
                   std::u16string_view,
                   const std::u16string_view,
                   std::u32string_view,
                   const std::u32string_view,
                   std::vector<char>,
                   const std::vector<char>,
                   std::vector<int>,
                   const std::vector<int>) {
    static constexpr char message[] = "hello, world!";

    using char_t                             = TestType::value_type;
    static constexpr std::size_t size        = sizeof(message);
    static constexpr std::size_t total_bytes = size * sizeof(char_t);

    using array_t = std::array<char_t, size>;

    STATIC_REQUIRE(!std::is_const_v<char_t>);
    STATIC_REQUIRE(sizeof(array_t) == total_bytes);

    array_t str{};

    std::ranges::copy(message, str.data());

    TestType range(str.data(), str.data() + size);

    using expected_buffer_t =
        buffer_type_t<std::remove_pointer_t<decltype(range.data())>>;

    SECTION("make_buffer(Range&)") {
        auto buffer = sendosio::make_buffer(range);

        STATIC_REQUIRE(std::same_as<expected_buffer_t, decltype(buffer)>);

        REQUIRE(buffer.data() == range.data());
        REQUIRE(buffer.size() == size * sizeof(char_t));
    }

    SECTION("make_buffer(Range&&)") {
        using namespace std::ranges::views;

        // all(range) needs to return an object for this test to actually be validating
        // make_buffer of an rvalue
        STATIC_REQUIRE(std::is_object_v<all_t<TestType&>>);

        auto buffer = sendosio::make_buffer(all(range));

        STATIC_REQUIRE(std::same_as<expected_buffer_t, decltype(buffer)>);

        REQUIRE(buffer.data() == range.data());
        REQUIRE(buffer.size() == size * sizeof(char_t));
    }

    SECTION("make_buffer(Range&, at_least_half(size))") {
        constexpr std::size_t max_size = at_least_half(total_bytes);

        auto buffer = sendosio::make_buffer(range, max_size);

        STATIC_REQUIRE(std::same_as<expected_buffer_t, decltype(buffer)>);

        REQUIRE(buffer.data() == range.data());
        REQUIRE(buffer.size() == max_size);
    }

    SECTION("make_buffer(Range&, 2 * size)") {
        auto buffer = sendosio::make_buffer(range, total_bytes * 2);

        STATIC_REQUIRE(std::same_as<expected_buffer_t, decltype(buffer)>);

        REQUIRE(buffer.data() == range.data());
        REQUIRE(buffer.size() == total_bytes);
    }
}

TEST_CASE("make_buffer(Range) and make_buffer(Range, max_size) reject invalid ranges",
          "[sendosio::make_buffer]") {
    using sendosio::make_buffer_t;
    using std::ranges::views::all_t;

    // lvalue owning range is OK
    STATIC_REQUIRE(std::invocable<make_buffer_t, std::vector<int>&>);
    STATIC_REQUIRE(std::invocable<make_buffer_t, std::vector<int>&, std::size_t>);
    STATIC_REQUIRE(std::invocable<make_buffer_t, const std::vector<int>&>);
    STATIC_REQUIRE(std::invocable<make_buffer_t, const std::vector<int>&, std::size_t>);

    // rvalue owning range is not OK--the resulting buffer would dangle
    STATIC_REQUIRE(!std::invocable<make_buffer_t, std::vector<int>>);
    STATIC_REQUIRE(!std::invocable<make_buffer_t, std::vector<int>, std::size_t>);
    STATIC_REQUIRE(!std::invocable<make_buffer_t, std::vector<int>&&>);
    STATIC_REQUIRE(!std::invocable<make_buffer_t, std::vector<int>&&, std::size_t>);
    STATIC_REQUIRE(!std::invocable<make_buffer_t, const std::vector<int>&&>);
    STATIC_REQUIRE(!std::invocable<make_buffer_t, const std::vector<int>&&, std::size_t>);

    // borrowed ranges are fine in all value categories
    STATIC_REQUIRE(std::invocable<make_buffer_t, std::span<int>>);
    STATIC_REQUIRE(std::invocable<make_buffer_t, std::span<int>, std::size_t>);
    STATIC_REQUIRE(std::invocable<make_buffer_t, std::span<int>&>);
    STATIC_REQUIRE(std::invocable<make_buffer_t, std::span<int>&, std::size_t>);
    STATIC_REQUIRE(std::invocable<make_buffer_t, const std::span<int>&>);
    STATIC_REQUIRE(std::invocable<make_buffer_t, const std::span<int>&, std::size_t>);
    STATIC_REQUIRE(std::invocable<make_buffer_t, std::span<int>&&>);
    STATIC_REQUIRE(std::invocable<make_buffer_t, std::span<int>&&, std::size_t>);
    STATIC_REQUIRE(std::invocable<make_buffer_t, const std::span<int>&&>);
    STATIC_REQUIRE(std::invocable<make_buffer_t, const std::span<int>&&, std::size_t>);

    // borrowing from an lvalue vector is fine
    STATIC_REQUIRE(std::invocable<make_buffer_t, all_t<std::vector<int>&>>);
    STATIC_REQUIRE(std::invocable<make_buffer_t, all_t<std::vector<int>&>, std::size_t>);
    // but borrowing from an rvalue vecrtor is rejected
    STATIC_REQUIRE(!std::invocable<make_buffer_t, all_t<std::vector<int>>>);
    STATIC_REQUIRE(!std::invocable<make_buffer_t, all_t<std::vector<int>>, std::size_t>);
}

} // namespace
