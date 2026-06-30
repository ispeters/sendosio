// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/sendosio/config.hpp>
#include <catch2/catch_all.hpp>
#include <beman/sendosio/todo.hpp>

TEST_CASE("todo", "[sendosio::todo]") {
    const bool todo = true;
    CHECK(todo);
}
