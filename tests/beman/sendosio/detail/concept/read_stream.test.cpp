// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "beman/sendosio/detail/concept/read_stream.hpp"

#include <catch2/catch_all.hpp>

#include <beman/sendosio/detail/buffers.hpp>
#include <beman/sendosio/detail/vendor/execution.hpp>

#include <exception>
#include <system_error>

namespace {

namespace sendosio = beman::sendosio;
namespace ex       = sendosio::ex;
namespace exec     = sendosio::exec;

TEMPLATE_TEST_CASE(
    "a read_some with that returns a fixed just sender satisfies read_stream",
    "[sendosio::read_stream]",
    // always succeed
    decltype(ex::just(std::error_code(), std::size_t())),
    // always fail with a std::error_code
    decltype(ex::just_error(std::error_code())),
    // always fail with a std::exception_ptr
    decltype(ex::just_error(std::exception_ptr())),
    // always report stopped
    decltype(ex::just_stopped())) {

    struct stream {
        TestType read_some(sendosio::mutable_buffer);
    };

    STATIC_REQUIRE(sendosio::read_stream<stream>);
}

TEMPLATE_TEST_CASE(
    "a read_some that requires an environment query and then succeeds satisifies "
    "read_stream",
    "[sendosio::read_stream]",
    ex::get_start_scheduler_t,
    ex::get_stop_token_t,
    exec::get_frame_allocator_t) {

    struct stream {
        auto read_some(sendosio::mutable_buffer) {
            return ex::read_env(TestType()) | ex::let_value([](auto&) noexcept {
                       return ex::just(std::error_code(), std::size_t());
                   });
        }
    };

    STATIC_REQUIRE(sendosio::read_stream<stream>);
}

} // namespace
