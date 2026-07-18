// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef BEMAN_SENDOSIO_DETAIL_VENDOR_STDEXEC_HPP
#define BEMAN_SENDOSIO_DETAIL_VENDOR_STDEXEC_HPP

#include <beman/sendosio/config.hpp>

// Only does anything in non-module builds. In module builds, the sole owner
// of these includes is detail/vendor/stdexec.cppm; everything else reaches
// stdexec via `import beman.sendosio;` and the `ex`/`exec` aliases it
// exports, never by #including stdexec/exec headers directly.
#if !BEMAN_SENDOSIO_USE_MODULES()

    #include <exec/get_frame_allocator.hpp>

    #include <stdexec/execution.hpp>

namespace beman::sendosio {
namespace ex   = STDEXEC;
namespace exec = ::exec;
} // namespace beman::sendosio

#endif

#endif
