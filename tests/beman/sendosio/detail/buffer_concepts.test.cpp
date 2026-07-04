// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <catch2/catch_all.hpp>

#include <beman/sendosio/detail/buffers.hpp>

#include <array>

namespace {

namespace sendosio = beman::sendosio;

TEST_CASE(
    "(an array of) const_buffer and mutable_buffer both satisfy const_buffer_sequence",
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

} // namespace
