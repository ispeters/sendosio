# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

if(BEMAN_SENDOSIO_USE_MODULES)
    target_sources(
        beman.sendosio
        PUBLIC
            FILE_SET CXX_MODULES
                FILES
                    sendosio.cppm
                    detail/buffers.cppm
                    detail/buffers/make_buffer.cppm
            FILE_SET HEADERS
                FILES
                    config.hpp
                    sendosio.hpp
                    todo.hpp
                    "${PROJECT_BINARY_DIR}/include/beman/sendosio/config_generated.hpp"
    )
else()
    target_sources(
        beman.sendosio
        PUBLIC
            FILE_SET HEADERS
                FILES
                    config.hpp
                    sendosio.hpp
                    todo.hpp
                    "${PROJECT_BINARY_DIR}/include/beman/sendosio/config_generated.hpp"
    )
endif()
