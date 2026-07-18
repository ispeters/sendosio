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
    #endif

namespace beman::sendosio {

template <const_buffer_sequence Buffers>
struct sliced {
    using iterator_type = decltype(sendosio::begin(std::declval<const Buffers&>()));
    using buffer_type   = sendosio::buffer_type<Buffers>;

    constexpr explicit sliced(const Buffers& seq,
                              std::size_t    offset,
                              std::size_t    length) noexcept
        : data_(seq, offset, length) {}

    constexpr data_view<iterator_type> data() const noexcept { return data_; }

    constexpr void remove_prefix(std::size_t prefix) noexcept {
        // update begin_, skip_front_, and seq_length_ to account for having removed
        // prefix bytes from the front of the buffer sequence
        data_.update_front(data(), prefix);
    }

  private:
    data_view<iterator_type> data_;
};

template <class Buffers>
sliced(const Buffers&) -> sliced<Buffers>;

} // namespace beman::sendosio

#endif // BEMAN_SENDOSIO_USE_MODULES() &&
       // !defined(BEMAN_SENDOSIO_INCLUDED_FROM_INTERFACE_UNIT)

#endif // BEMAN_SENDOSIO_DETAIL_BUFFERS_CONSUMING_BUFFERS_HPP
