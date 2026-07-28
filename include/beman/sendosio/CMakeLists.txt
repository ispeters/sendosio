# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

if(BEMAN_SENDOSIO_USE_MODULES)
    target_sources(
        beman.sendosio
        PUBLIC
            FILE_SET CXX_MODULES FILES sendosio.cppm
            FILE_SET HEADERS
                FILES
                    detail/buffers/buffer_copy.hpp
                    detail/buffers/buffer_slice.hpp
                    detail/buffers/consuming_buffers.hpp
                    detail/buffers/make_buffer.hpp
                    detail/buffers/slice_of.hpp
                    detail/buffers.hpp
                    detail/concept/read_stream.hpp
                    detail/contracts.hpp
                    detail/read.hpp
                    config.hpp
                    sendosio.hpp
                    "${PROJECT_BINARY_DIR}/include/beman/sendosio/config_generated.hpp"
    )
else()
    target_sources(
        beman.sendosio
        PUBLIC
            FILE_SET HEADERS
                FILES
                    detail/buffers/buffer_copy.hpp
                    detail/buffers/buffer_slice.hpp
                    detail/buffers/consuming_buffers.hpp
                    detail/buffers/make_buffer.hpp
                    detail/buffers/slice_of.hpp
                    detail/buffers.hpp
                    detail/concept/read_stream.hpp
                    detail/contracts.hpp
                    detail/read.hpp
                    config.hpp
                    sendosio.hpp
                    "${PROJECT_BINARY_DIR}/include/beman/sendosio/config_generated.hpp"
    )
endif()
