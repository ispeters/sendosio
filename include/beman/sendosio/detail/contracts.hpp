// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_SENDOSIO_DETAIL_CONTRACTS_HPP
#define BEMAN_SENDOSIO_DETAIL_CONTRACTS_HPP

#include <beman/sendosio/config.hpp>

#if BEMAN_SENDOSIO_USE_CONTRACTS()
    #include <contracts>

    #define BEMAN_SENDOSIO_PRE(...) pre(__VA_ARGS__)
    #define BEMAN_SENDOSIO_POST(...) post(__VA_ARGS__)
    #define BEMAN_SENDOSIO_CONTRACT_ASSERT(...) contract_assert(__VA_ARGS__)
#else
    #include <cassert>

    #define BEMAN_SENDOSIO_PRE(...)
    #define BEMAN_SENDOSIO_POST(...)
    // TODO: consider whether this is wise
    #define BEMAN_SENDOSIO_CONTRACT_ASSERT(...) assert(__VA_ARGS__)
#endif

#endif
