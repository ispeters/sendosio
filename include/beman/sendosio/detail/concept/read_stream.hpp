// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_SENDOSIO_DETAIL_CONCEPT_READ_STREAM_HPP
#define BEMAN_SENDOSIO_DETAIL_CONCEPT_READ_STREAM_HPP

#include <beman/sendosio/config.hpp>

#if BEMAN_SENDOSIO_USE_MODULES() && !defined(BEMAN_SENDOSIO_INCLUDED_FROM_INTERFACE_UNIT)

import beman.sendosio;

#else

    #if !BEMAN_SENDOSIO_USE_MODULES()
        #include <beman/sendosio/detail/buffers.hpp>
        #include <stdexec/execution.hpp>

        #include <concepts>
        #include <cstddef>
        #include <ranges>
        #include <string_view>
        #include <type_traits>
    #endif

namespace beman::sendosio {

// Capy defines a "mutable_buffer_archetype" that is just an alias for mutable_buffer when
// __clang__ is defined, but is otherwise a type that converts to mutable_buffer and
// const_buffer but cannot be instantiated. I don't understand the complexity so I'm not
// adding it, yet.
template <class Stream>
concept read_stream = requires(Stream& stream, mutable_buffer buffer) {
    { stream.read_some(buffer) } -> STDEXEC::sender;
};
} // namespace beman::sendosio

#endif

#endif
