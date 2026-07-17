// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_SENDOSIO_DETAIL_CONCEPT_SLICE_HPP
#define BEMAN_SENDOSIO_DETAIL_CONCEPT_SLICE_HPP

#include <beman/sendosio/config.hpp>

#if BEMAN_SENDOSIO_USE_MODULES() && !defined(BEMAN_SENDOSIO_INCLUDED_FROM_INTERFACE_UNIT)

import beman.sendosio;

#else

    #if !BEMAN_SENDOSIO_USE_MODULES()
        #include <beman/sendosio/detail/buffers.hpp>

        #include <cstddef>
    #endif

namespace beman::sendosio {
template <class Slice>
concept slice = //
    requires(Slice& sl, const Slice& csl, std::size_t n) {
        { csl.data() } -> const_buffer_sequence;
        sl.remove_prefix(n);
    };

template <class Slice>
concept mutable_slice = //
    slice<Slice>        //
    && requires(const Slice& csl) {
           { csl.data() } -> mutable_buffer_sequence;
       };

} // namespace beman::sendosio

#endif // BEMAN_SENDOSIO_USE_MODULES() &&
       // !defined(BEMAN_SENDOSIO_INCLUDED_FROM_INTERFACE_UNIT)

#endif // BEMAN_SENDOSIO_DETAIL_CONCEPT_SLICE_HPP
