// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_SENDOSIO_DETAIL_BUFFERS_CONSUMING_BUFFERS_HPP
#define BEMAN_SENDOSIO_DETAIL_BUFFERS_CONSUMING_BUFFERS_HPP

#include <beman/sendosio/config.hpp>

#if BEMAN_SENDOSIO_USE_MODULES() && !defined(BEMAN_SENDOSIO_INCLUDED_FROM_INTERFACE_UNIT)

import beman.sendosio;

#else

    #if !BEMAN_SENDOSIO_USE_MODULES()
        #include <beman/sendosio/detail/buffers.hpp>
        #include <beman/sendosio/detail/buffers/slice_of.hpp>

        #include <cstddef>
        #include <limits>
        #include <utility>
    #endif

namespace beman::sendosio {

template <const_buffer_sequence Buffers>
class consuming_buffers {
    using iterator_type = decltype(sendosio::begin(std::declval<const Buffers&>()));

    slice_of<iterator_type> data_;

  public:
    using buffer_type = sendosio::buffer_type<Buffers>;

    constexpr explicit consuming_buffers(const Buffers& seq) noexcept
        : data_(seq, 0, (std::numeric_limits<std::size_t>::max)()) {}

    // this disables construction from rvalue buffer sequences, which prevents
    // constructing a view over a dangling reference; this is a useful safety feature but
    // I think it's too strict--I'd like to support rvalue views that are "safe" and I'm
    // pretty sure there's a concept in std::ranges that lets me do that
    constexpr explicit consuming_buffers(const Buffers&&) noexcept = delete;

    constexpr slice_of<iterator_type> data() const noexcept { return data_; }

    constexpr void consume(std::size_t prefix) noexcept {
        // update begin_, skip_front_, and seq_length_ to account for having removed
        // prefix bytes from the front of the buffer sequence
        data_.update_front(data(), prefix);
    }
};

template <class Buffers>
consuming_buffers(const Buffers&) -> consuming_buffers<Buffers>;

} // namespace beman::sendosio

#endif // BEMAN_SENDOSIO_USE_MODULES() &&
       // !defined(BEMAN_SENDOSIO_INCLUDED_FROM_INTERFACE_UNIT)

#endif // BEMAN_SENDOSIO_DETAIL_BUFFERS_CONSUMING_BUFFERS_HPP
