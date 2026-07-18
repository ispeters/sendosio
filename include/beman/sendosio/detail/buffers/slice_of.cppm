// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
module;

#include <beman/sendosio/detail/contracts.hpp>

export module beman.sendosio:detail.buffers.slice_of;

import std;
import :detail.buffers.make_buffer;
import :detail.buffers;

#define BEMAN_SENDOSIO_INCLUDED_FROM_INTERFACE_UNIT
export {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"
#include <beman/sendosio/detail/buffers/slice_of.hpp>
#pragma clang diagnostic pop
}
