// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

export module beman.sendosio:detail.buffers.buffer_copy;

import std;
import :detail.buffers;
import :detail.buffers.buffer_slice;
import :detail.buffers.slice_of;

#define BEMAN_SENDOSIO_INCLUDED_FROM_INTERFACE_UNIT
export {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"
#include <beman/sendosio/detail/buffers/buffer_copy.hpp>
#pragma clang diagnostic pop
}
