// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_SENDOSIO_SENDOSIO_HPP
#define BEMAN_SENDOSIO_SENDOSIO_HPP

#include <beman/sendosio/config.hpp>

#if BEMAN_SENDOSIO_USE_MODULES() && !defined(BEMAN_SENDOSIO_INCLUDED_FROM_INTERFACE_UNIT)

import beman.sendosio;

#else

    #include <beman/sendosio/detail/buffers.hpp>
    #include <beman/sendosio/detail/buffers/buffer_slice.hpp>
    #include <beman/sendosio/detail/buffers/consuming_buffers.hpp>
    #include <beman/sendosio/detail/buffers/make_buffer.hpp>
    #include <beman/sendosio/detail/buffers/slice_of.hpp>

#endif // BEMAN_SENDOSIO_USE_MODULES() &&
       // !defined(BEMAN_SENDOSIO_INCLUDED_FROM_INTERFACE_UNIT)

#endif // BEMAN_SENDOSIO_SENDOSIO_HPP
