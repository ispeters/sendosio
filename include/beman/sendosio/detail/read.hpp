// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_SENDOSIO_DETAIL_READ_HPP
#define BEMAN_SENDOSIO_DETAIL_READ_HPP

#include <beman/sendosio/config.hpp>

#if BEMAN_SENDOSIO_USE_MODULES() && !defined(BEMAN_SENDOSIO_INCLUDED_FROM_INTERFACE_UNIT)

import beman.sendosio;

#else

    #if !BEMAN_SENDOSIO_USE_MODULES()
        #include <beman/sendosio/detail/buffers.hpp>
        #include <beman/sendosio/detail/concept/read_stream.hpp>
        #include <stdexec/execution.hpp>
    #endif

namespace beman::sendosio {
namespace read_detail {

namespace ex = STDEXEC;

template <class Stream, class Buffers, class Receiver>
struct op_state {
    using operation_state_concept = ex::operation_state_tag;

    void run() & noexcept {}

    op_state(Stream&& stream, Buffers&& buffers, Receiver rcvr)
        : stream_(std::move(stream)),
          buffers_(std::move(buffers)),
          rcvr_(std::move(rcvr)) {}

  private:
    Stream   stream_;
    Buffers  buffers_;
    Receiver rcvr_;
};

struct read_impl : ex::__sexpr_defaults {
    template <class Sender, class Env>
    static consteval auto __get_completion_signatures() noexcept {
        // TODO: this is certainly wrong
        return ex::completion_signatures<ex::set_value_t(std::error_code, std::size_t)>{};
    }

    static constexpr auto __get_state =
        []<class Self, class Receiver>(Self&& self, Receiver rcvr) noexcept /*TODO*/ {
            auto& [tag, data]       = self;
            auto& [stream, buffers] = data;

            return op_state(std::forward_like<Self>(stream),
                            std::forward_like<Self>(buffers),
                            std::move(rcvr));
        };

    static constexpr auto __start = [](auto& state) noexcept { state.run(); };
};

struct read_t {
    template <read_stream Stream, mutable_buffer_sequence Buffers>
    auto operator()(Stream& stream, Buffers buffers) const {
        return ex::__make_sexpr<read_t>(
            ex::__tuple{std::addressof(stream), std::move(buffers)});
    }
};

} // namespace read_detail

using read_detail::read_t;

inline constexpr read_t read{};

} // namespace beman::sendosio

template <>
struct STDEXEC::__sexpr_impl<beman::sendosio::read_t>
    : beman::sendosio::read_detail::read_impl {};

#endif

#endif
