module;

#include <exec/get_frame_allocator.hpp>

#include <stdexec/execution.hpp>

#include <cassert>

export module beman.sendosio;

import std;

#define BEMAN_SENDOSIO_INCLUDED_FROM_INTERFACE_UNIT
export {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"
#include <beman/sendosio/detail/buffers.hpp>
#include <beman/sendosio/detail/buffers/buffer_copy.hpp>
#include <beman/sendosio/detail/buffers/buffer_slice.hpp>
#include <beman/sendosio/detail/buffers/consuming_buffers.hpp>
#include <beman/sendosio/detail/buffers/make_buffer.hpp>
#include <beman/sendosio/detail/buffers/slice_of.hpp>
#include <beman/sendosio/detail/concept/read_stream.hpp>
#include <beman/sendosio/detail/contracts.hpp>
#include <beman/sendosio/detail/read.hpp>
#include <beman/sendosio/sendosio.hpp>
#pragma clang diagnostic pop
}
