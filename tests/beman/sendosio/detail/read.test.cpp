// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "beman/sendosio/detail/read.hpp"

#include <catch2/catch_all.hpp>

#include <beman/sendosio/detail/buffers/make_buffer.hpp>
#include <beman/sendosio/detail/vendor/execution.hpp>

#include <array>

namespace {

namespace sendosio = beman::sendosio;
namespace ex       = sendosio::ex;

struct strstream {
    std::string data;

    ex::sender auto read_some(sendosio::mutable_buffer buffer) {
        return ex::just(std::error_code{}, (std::min)(data.size(), buffer.size()));
    }
};

TEST_CASE("read returns a sender", "[sendosio::read]") {
    strstream            stream{"hello, world!"};
    std::array<char, 20> buffer;

    auto sndr = sendosio::read(stream, sendosio::make_buffer(buffer));
    STATIC_REQUIRE(ex::sender<decltype(sndr)>);
}

} // namespace
