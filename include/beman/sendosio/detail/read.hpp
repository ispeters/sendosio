// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_SENDOSIO_DETAIL_READ_HPP
#define BEMAN_SENDOSIO_DETAIL_READ_HPP

#include <beman/sendosio/config.hpp>

#if BEMAN_SENDOSIO_USE_MODULES() && !defined(BEMAN_SENDOSIO_INCLUDED_FROM_INTERFACE_UNIT)

import beman.sendosio;

#else

    #include <beman/sendosio/detail/buffers.hpp>
    #include <beman/sendosio/detail/buffers/consuming_buffers.hpp>
    #include <beman/sendosio/detail/concept/read_stream.hpp>
    #include <beman/sendosio/detail/vendor/execution.hpp>

namespace beman::sendosio {
namespace read_detail {

template <class Stream, class Buffers, class Receiver>
struct op_state {
    static_assert(std::is_pointer_v<Stream>);

    // I think this does not actually need to be a real operation state because it's owned
    // by a basic_operation_state
    //
    // using operation_state_concept = ex::operation_state_tag;

    op_state(Stream stream, Buffers&& buffers, Receiver rcvr)
        : stream_(stream),
          buffers_(std::move(buffers)), // TODO: move?
          consuming_(buffers_),
          total_size_(buffer_size(buffers)),
          rcvr_(std::move(rcvr)) {} // TODO: move?

    op_state(op_state&&) = delete;

    ~op_state() {}

    void run() & noexcept { start_next_iteration(); }

  private:
    Stream                     stream_;
    Buffers                    buffers_;
    consuming_buffers<Buffers> consuming_;
    slice_type<Buffers>        consuming_data_;
    const std::size_t          total_size_;
    std::size_t                total_read_{};
    Receiver                   rcvr_;

    struct loop_rcvr {
        using receiver_concept = ex::receiver_tag;

        void set_value(std::error_code ec, std::size_t n) && noexcept {
            self->consuming_.consume(n);
            self->total_read_ += n;

            if (ec && self->total_read_ < self->total_size_) {
                // there's an error and we've read less than everything so report the
                // error
                self->destroy_loop_op_and_invoke([ec](op_state* self) noexcept {
                    ex::set_value(std::move(self->rcvr_), ec, self->total_read_);
                });
            } else {
                // loop...
                self->destroy_loop_op_and_invoke(
                    [](op_state* self) noexcept { self->start_next_iteration(); });
            }
        }

        void set_error(std::error_code ec) && noexcept {
            // maybe the semantic should be this->set_value(ec, 0)?
            // Capy doesn't actually support this error channel...
            self->destroy_loop_op_and_invoke([ec](op_state* self) noexcept {
                ex::set_error(std::move(self->rcvr_), ec);
            });
        }

        void set_error(std::exception_ptr e) && noexcept {
            self->destroy_loop_op_and_invoke([&e](op_state* self) noexcept {
                ex::set_error(std::move(self->rcvr_), std::move(e));
            });
        }

        void set_stopped() && noexcept {
            self->destroy_loop_op_and_invoke(
                [](op_state* self) noexcept { ex::set_stopped(std::move(self->rcvr_)); });
        }

        ex::env_of_t<Receiver> get_env() const noexcept { return self->rcvr_.get_env(); }

        op_state* self;
    };

    using read_some_t = decltype(stream_->read_some(consuming_data_));
    using loop_op_t   = ex::connect_result_t<read_some_t, loop_rcvr>;

    union {
        loop_op_t loop_op_;
    };

    void start_next_iteration() noexcept {
        // invariant: loop_op_ does not need destroying on entry

        if (total_read_ < total_size_) {
            consuming_data_ = consuming_.data();

            // TODO: consider whether we need a trampoline scheduler...
            auto* child = new ((void*)std::addressof(loop_op_)) loop_op_t(
                ex::connect(stream_->read_some(consuming_data_), loop_rcvr{this}));

            child->start();
        } else {
            ex::set_value(std::move(rcvr_), std::error_code(), total_read_);
        }
    }

    template <class Func>
    void destroy_loop_op_and_invoke(Func func) noexcept {
        std::destroy_at(std::addressof(loop_op_));
        std::move(func)(this);
    }
};

struct read_impl : ex::__sexpr_defaults {
    template <class Sender, class Env>
    static consteval auto __get_completion_signatures() noexcept {
        // TODO: this is maximal; with more work, I could report what my stream's
        //       read_some can return, which may be less
        return ex::completion_signatures<ex::set_value_t(std::error_code, std::size_t),
                                         ex::set_error_t(std::error_code),
                                         ex::set_error_t(std::exception_ptr),
                                         ex::set_stopped_t()>{};
    }

    static constexpr auto __get_state =
        []<class Self, class Receiver>(Self&& self, Receiver rcvr) noexcept /*TODO*/ {
            auto& [tag, data]       = self;
            auto& [stream, buffers] = data;

            return op_state(stream, // stream is a pointer
                            std::forward_like<Self>(buffers),
                            std::move(rcvr));
        };

    static constexpr auto __start = [](auto& state) noexcept { state.run(); };
};

struct read_t {
    template <read_stream Stream, mutable_buffer_sequence Buffers>
    auto operator()(Stream& stream, Buffers buffers) const {
        return ex::__make_sexpr<read_t>(ex::__tuple{std::addressof(stream), buffers});
    }
};

} // namespace read_detail

using read_detail::read_t;

inline constexpr read_t read{};

} // namespace beman::sendosio

template <>
struct beman::sendosio::ex::__sexpr_impl<beman::sendosio::read_t>
    : beman::sendosio::read_detail::read_impl {};

#endif

#endif
