// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_SENDOSIO_DETAIL_BUFFERS_BUFFER_COPY_HPP
#define BEMAN_SENDOSIO_DETAIL_BUFFERS_BUFFER_COPY_HPP

#include <beman/sendosio/config.hpp>

#if BEMAN_SENDOSIO_USE_MODULES() && !defined(BEMAN_SENDOSIO_INCLUDED_FROM_INTERFACE_UNIT)

import beman.sendosio;

#else

    #if !BEMAN_SENDOSIO_USE_MODULES()
        #include <beman/sendosio/detail/buffers.hpp>
        #include <beman/sendosio/detail/buffers/buffer_slice.hpp>
        #include <beman/sendosio/detail/buffers/consuming_buffers.hpp>
        #include <beman/sendosio/detail/buffers/slice_of.hpp>
        #include <beman/sendosio/detail/contracts.hpp>

        #include <algorithm>
        #include <cstddef>
        #include <cstring>
        #include <ranges>
    #endif

namespace beman::sendosio {

namespace buffer_copy_detail {

struct buffer_copy_t {
    template <mutable_buffer_sequence MB, const_buffer_sequence CB>
    constexpr std::size_t
    operator()(const MB& dest, const CB& src, std::size_t at_most) const noexcept {
        // limit dest and src to at_most bytes and then delegate to the implementation
        // that copies as many bytes as possible from src to dest
        return (*this)(buffer_slice(dest, 0, at_most), buffer_slice(src, 0, at_most));
    }

    template <mutable_buffer_sequence MB, const_buffer_sequence CB>
    constexpr std::size_t operator()(const MB& dest, const CB& src) const noexcept {
        auto limit = (std::min)(buffer_size(dest), buffer_size(src));
        return impl(buffer_slice(dest, 0, limit), buffer_slice(src, 0, limit));
    }

  private:
    template <class T, bool Single>
    using slice_of = slice_of_detail::slice_of<T, Single>;

    template <class MB, bool SingleMB, class CB, bool SingleCB>
    constexpr std::size_t impl(slice_of<MB, SingleMB> dslice,
                               slice_of<CB, SingleCB> sslice) const noexcept {
        // invariant: buffer_size(dslice) == buffer_size(sslice) and we're copying all the
        // bytes
        std::size_t       bytes_copied = 0;
        consuming_buffers dest_consumer(dslice);
        consuming_buffers src_consumer(sslice);

        while (true) {
            const auto dest_data = dest_consumer.data();
            const auto src_data  = src_consumer.data();

            static_assert(std::same_as<std::ranges::range_value_t<decltype(dest_data)>,
                                       mutable_buffer>);
            static_assert(
                std::convertible_to<std::ranges::range_value_t<decltype(src_data)>,
                                    const_buffer>);

            if (std::ranges::empty(src_data)) {
                // we're out of bytes to copy so we're done; we had better have filled up
                // dest, too, since it's an invariant that the two buffers are of the same
                // size
                BEMAN_SENDOSIO_CONTRACT_ASSERT(std::ranges::empty(dest_data));
                return bytes_copied;
            }

            const mutable_buffer dest = *std::ranges::begin(dest_data);
            const const_buffer   src  = *std::ranges::begin(src_data);

            // perform the actual copy; the overall buffer sequences must have the same
            // size, but they may be composed of buffers of arbitrary size so each copy
            // must ensure it doesn't overrun the end of a buffer
            const auto bytes = (std::min)(dest.size(), src.size());
            std::memcpy(dest.data(), src.data(), bytes);

            // this is the forward-progress guarantee; if we ever have bytes == 0 then
            // we'll loop forever
            BEMAN_SENDOSIO_CONTRACT_ASSERT(bytes > 0);

            // these calls to consume ought to empty dest, src, or both, advancing begin
            // on the corresponding data() view
            dest_consumer.consume(bytes);
            src_consumer.consume(bytes);
            // make sure our return value gets updated correctly
            bytes_copied += bytes;
        }
    }
};

} // namespace buffer_copy_detail

using buffer_copy_detail::buffer_copy_t;

inline constexpr buffer_copy_t buffer_copy{};

} // namespace beman::sendosio

#endif // BEMAN_SENDOSIO_USE_MODULES() &&
       // !defined(BEMAN_SENDOSIO_INCLUDED_FROM_INTERFACE_UNIT)

#endif // BEMAN_SENDOSIO_DETAIL_BUFFERS_BUFFER_COPY_HPP
