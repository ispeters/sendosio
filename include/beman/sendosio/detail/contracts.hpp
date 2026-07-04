// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_SENDOSIO_DETAIL_CONTRACTS_HPP
#define BEMAN_SENDOSIO_DETAIL_CONTRACTS_HPP

#include <beman/sendosio/config.hpp>

#if BEMAN_SENDOSIO_USE_CONTRACTS()
    #include <contracts>
#else
    // TODO: consider whether this is wise
    #include <cassert>
#endif

#endif
