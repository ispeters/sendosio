// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef BEMAN_SENDOSIO_DETAIL_VENDOR_STDEXEC_HPP
#define BEMAN_SENDOSIO_DETAIL_VENDOR_STDEXEC_HPP

#include <beman/sendosio/config.hpp>

#if BEMAN_SENDOSIO_USE_MODULES() && !defined(BEMAN_SENDOSIO_INCLUDED_FROM_INTERFACE_UNIT)

import beman.sendosio;

#else

// In module builds, these includes live in sendosio.cppm, but this header is still needed
// for the `ex`/`exec` aliases it exports.
    #if !BEMAN_SENDOSIO_USE_MODULES()

        #include <exec/get_frame_allocator.hpp>

        #include <stdexec/execution.hpp>

    #endif

namespace beman::sendosio {
namespace ex   = STDEXEC;
namespace exec = ::exec;
} // namespace beman::sendosio

#endif

#endif
