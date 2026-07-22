// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_SENDOSIO_DETAIL_CONCEPT_READ_STREAM_HPP
#define BEMAN_SENDOSIO_DETAIL_CONCEPT_READ_STREAM_HPP

#include <beman/sendosio/config.hpp>

#if BEMAN_SENDOSIO_USE_MODULES() && !defined(BEMAN_SENDOSIO_INCLUDED_FROM_INTERFACE_UNIT)

import beman.sendosio;

#else

    #include <beman/sendosio/detail/buffers.hpp>
    #include <beman/sendosio/detail/vendor/execution.hpp>

    #if !BEMAN_SENDOSIO_USE_MODULES()

        #include <cstddef>
        #include <exception>
        #include <memory_resource>
        #include <system_error>
    #endif

namespace beman::sendosio {

namespace read_stream_detail {

namespace ex = beman::sendosio::ex;

struct io_env {
    ex::inline_scheduler query(ex::get_start_scheduler_t) const noexcept;

    ex::inplace_stop_token query(ex::get_stop_token_t) const noexcept;

    std::pmr::polymorphic_allocator<> query(exec::get_frame_allocator_t) const noexcept;
};

struct io_result_receiver {
    using receiver_concept = ex::receiver_tag;

    // this mirrors Capy's
    //
    //     awaitable_decomposes_to<
    //        decltype(stream.read_some(buffers)), std::error_code, std::size_t>
    //
    // constraint in boost::capy::ReadStream directly
    void set_value(std::error_code, std::size_t) && noexcept;

    // perhaps, to enable something like
    //
    //     `std::execution::task<std::pair<std::error_code, std::size_t>>`
    //
    // to be a valid io_result_sender, there should be a set_value overload here that
    // accepts "anything that decomposes to [error_code, size_t]", which would more
    // directly mirror what Capy does

    // this seems useful
    void set_error(std::error_code) && noexcept;

    // this is required if read_some-senders are going to be allowed to allocate
    void set_error(std::exception_ptr) && noexcept;

    // this seems useful
    void set_stopped() && noexcept;

    io_env get_env() const noexcept;
};

} // namespace read_stream_detail

// Capy defines a "mutable_buffer_archetype" that is just an alias for mutable_buffer when
// __clang__ is defined, but is otherwise a type that converts to mutable_buffer and
// const_buffer but cannot be instantiated. I don't understand the complexity so I'm not
// adding it, yet.
template <class Stream>
concept read_stream = requires(Stream& stream, mutable_buffer buffer) {
    { stream.read_some(buffer) } -> ex::sender_to<read_stream_detail::io_result_receiver>;
};

} // namespace beman::sendosio

#endif

#endif
