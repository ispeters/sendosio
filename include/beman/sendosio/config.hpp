// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_SENDOSIO_CONFIG_HPP
#define BEMAN_SENDOSIO_CONFIG_HPP

#if !defined(__has_include) || __has_include(<beman/sendosio/config_generated.hpp>)
    #include <beman/sendosio/config_generated.hpp>
#else
    #define BEMAN_SENDOSIO_USE_MODULES() 0
    #define BEMAN_SENDOSIO_FORCE_NO_CONTRACTS() 1
    #define BEMAN_SENDOSIO_USE_CONTRACTS() 0
#endif

#endif
