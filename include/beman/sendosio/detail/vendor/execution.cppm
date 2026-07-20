// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
module;

#include <exec/get_frame_allocator.hpp>

#include <stdexec/execution.hpp>

export module beman.sendosio:detail.vendor.execution;

// Everything reachable through STDEXEC is attached to this partition's global
// module fragment: reachable to anything that imports this partition, but
// *not* visible for fresh qualified-name lookup -- reachability isn't
// visibility, and macros never cross an import boundary regardless. Each name
// beman.sendosio's implementation or tests need has to be individually
// re-exported here via a using-declaration. This file is the only place that
// should ever need to grow as more of stdexec's surface gets used.
export namespace beman::sendosio {
namespace ex {
using STDEXEC::completion_signatures;
using STDEXEC::get_start_scheduler_t;
using STDEXEC::get_stop_token_t;
using STDEXEC::inline_scheduler;
using STDEXEC::inplace_stop_token;
using STDEXEC::just;
using STDEXEC::just_error;
using STDEXEC::just_stopped;
using STDEXEC::let_value;
using STDEXEC::operation_state_tag;
using STDEXEC::read_env;
using STDEXEC::receiver_tag;
using STDEXEC::sender;
using STDEXEC::sender_to;
using STDEXEC::set_value_t;
// I wonder if these "hidden" symbols should be in a separate module fragment that is
// usable inside beman.sendosio, but not re-exported.
using STDEXEC::__make_sexpr;
using STDEXEC::__sexpr_defaults;
using STDEXEC::__sexpr_impl;
using STDEXEC::__tuple;
} // namespace ex

namespace exec {
using ::exec::get_frame_allocator_t;
} // namespace exec
} // namespace beman::sendosio
