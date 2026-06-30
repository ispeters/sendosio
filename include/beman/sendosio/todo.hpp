// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_SENDOSIO_TODO_HPP
#define BEMAN_SENDOSIO_TODO_HPP

#include <beman/sendosio/config.hpp>

#if BEMAN_SENDOSIO_USE_MODULES() && !defined(BEMAN_SENDOSIO_INCLUDED_FROM_INTERFACE_UNIT)

import beman.sendosio;

#else

namespace beman::sendosio {

// TODO

} // namespace beman::sendosio

#endif // BEMAN_SENDOSIO_USE_MODULES() &&
       // !defined(BEMAN_SENDOSIO_INCLUDED_FROM_INTERFACE_UNIT)

#endif // BEMAN_SENDOSIO_TODO_HPP
