// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
module;

#include <stdexec/execution.hpp>

export module beman.sendosio:detail.read;

import std;
import :detail.concepts.read_stream;

#define BEMAN_SENDOSIO_INCLUDED_FROM_INTERFACE_UNIT
export {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"
#include <beman/sendosio/detail/read.hpp>
#pragma clang diagnostic pop
}
