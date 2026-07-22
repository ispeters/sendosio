// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_SENDOSIO_DETAIL_CONTRACTS_HPP
#define BEMAN_SENDOSIO_DETAIL_CONTRACTS_HPP

#include <beman/sendosio/config.hpp>

#if BEMAN_SENDOSIO_USE_MODULES() && !defined(BEMAN_SENDOSIO_INCLUDED_FROM_INTERFACE_UNIT)

import beman.sendosio;

#else

    #if !BEMAN_SENDOSIO_USE_MODULES()

        #if BEMAN_SENDOSIO_USE_CONTRACTS()
            #include <contracts>
        #else
            // TODO: consider whether this is wise
            #include <cassert>
        #endif

    #endif // !BEMAN_SENDOSIO_USE_MODULES()

#endif // BEMAN_SENDOSIO_USE_MODULES() &&
       // !defined(BEMAN_SENDOSIO_INCLUDED_FROM_INTERFACE_UNIT)

#endif // BEMAN_SENDOSIO_DETAIL_CONTRACTS_HPP
