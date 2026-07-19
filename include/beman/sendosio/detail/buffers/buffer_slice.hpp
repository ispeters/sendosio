// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_SENDOSIO_DETAIL_BUFFERS_BUFFER_SLICE_HPP
#define BEMAN_SENDOSIO_DETAIL_BUFFERS_BUFFER_SLICE_HPP

#include <beman/sendosio/config.hpp>

#if BEMAN_SENDOSIO_USE_MODULES() && !defined(BEMAN_SENDOSIO_INCLUDED_FROM_INTERFACE_UNIT)

import beman.sendosio;

#else

    #if !BEMAN_SENDOSIO_USE_MODULES()
        #include <beman/sendosio/detail/buffers.hpp>
        #include <beman/sendosio/detail/buffers/make_buffer.hpp>
        #include <beman/sendosio/detail/buffers/slice_of.hpp>
        #include <beman/sendosio/detail/contracts.hpp>

        #include <cstddef>
        #include <iterator>
        #include <limits>
        #include <ranges>
        #include <utility>
    #endif

namespace beman::sendosio {

    #if 0
template <class Buffers>
using slice_type = std::conditional_t<
    std::convertible_to<Buffers, const_buffer>,
    buffer_type<Buffers>,
    slice_of<decltype(sendosio::begin(std::declval<const Buffers&>()))> >;
    #else
template <class Buffers>
using slice_type = slice_of<decltype(sendosio::begin(std::declval<const Buffers&>()))>;
    #endif

namespace buffer_slice_detail {

struct buffer_slice_t {
    template <const_buffer_sequence Buffers>
    constexpr slice_type<Buffers> operator()(
        const Buffers& seq,
        std::size_t    offset = 0,
        std::size_t length = (std::numeric_limits<std::size_t>::max)()) const noexcept {
    #if 0
        if constexpr (std::convertible_to<Buffers, const_buffer>) {
            return make_buffer(make_buffer(seq) + offset, length);
        } else {
    #endif
        using iterator_t = decltype(sendosio::begin(seq));
        return slice_of<iterator_t>(seq, offset, length);
    #if 0
        }
    #endif
    }

    // Capy deletes this overload to avoid creating dangling slices; I think it might be
    // too restrictive because rvalue views are usually safe, but I'll need tests to
    // confirm
    template <const_buffer_sequence Buffers>
    void operator()(const Buffers&&, std::size_t = 0, std::size_t = 0) const = delete;
};

} // namespace buffer_slice_detail

using buffer_slice_detail::buffer_slice_t;

inline constexpr buffer_slice_t buffer_slice{};

} // namespace beman::sendosio

#endif // BEMAN_SENDOSIO_USE_MODULES() &&
       // !defined(BEMAN_SENDOSIO_INCLUDED_FROM_INTERFACE_UNIT)

#endif // BEMAN_SENDOSIO_DETAIL_BUFFERS_BUFFER_SLICE_HPP
