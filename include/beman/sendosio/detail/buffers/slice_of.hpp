// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_SENDOSIO_DETAIL_BUFFERS_SLICE_OF_HPP
#define BEMAN_SENDOSIO_DETAIL_BUFFERS_SLICE_OF_HPP

#include <beman/sendosio/config.hpp>

#if BEMAN_SENDOSIO_USE_MODULES() && !defined(BEMAN_SENDOSIO_INCLUDED_FROM_INTERFACE_UNIT)

import beman.sendosio;

#else

    #if !BEMAN_SENDOSIO_USE_MODULES()
        #include <beman/sendosio/detail/buffers.hpp>
        #include <beman/sendosio/detail/buffers/make_buffer.hpp>
        #include <beman/sendosio/detail/contracts.hpp>

        #include <cstddef>
        #include <iterator>
        #include <limits>
        #include <ranges>
        #include <utility>
    #endif

namespace beman::sendosio {

template <const_buffer_sequence>
struct consuming_buffers;

namespace slice_of_detail {

template <std::ranges::view View>
constexpr auto buffer_sizes(View buffers) noexcept {
    return buffers | std::views::transform(std::ranges::size);
}

// heavily inspired by Claude Sonnet 5
// https://claude.ai/share/cb3523ee-45b9-4984-b6af-2c0da373206a
template <std::bidirectional_iterator Iterator, bool SingleBuffer>
struct slice_of : std::ranges::view_interface<slice_of<Iterator, SingleBuffer> > {
    using iterator_type = Iterator;

    using buffer_type = std::iter_value_t<iterator_type>;

    constexpr slice_of() noexcept = default;

    template <const_buffer_sequence Buffers>
        requires std::same_as<buffer_type, sendosio::buffer_type<Buffers> >
                     // make sure we don't stomp on the copy constructor
                     && (!std::same_as<slice_of, Buffers>)
    explicit constexpr slice_of(
        const Buffers& seq,
        std::size_t    offset = 0,
        std::size_t    length = (std::numeric_limits<std::size_t>::max)()) noexcept
        : begin_(sendosio::begin(seq)), end_(begin_) {
        // no point doing a bunch of work in update_front only to leave the range empty
        // anyway
        if (length > 0) {
            // this initializes begin_ and skip_front_, and drives seq_length_ to a
            // meaningless negative value
            initialize_front(sendosio::begin(seq), sendosio::end(seq), offset);

            // this depends on begin_ and skip_front_ being initialized, above; it updates
            // end_, skip_back_, and seq_length_ to the correct values
            initialize_back(sendosio::end(seq), length);
        }
    }

    struct const_iterator {
        using value_type        = std::iter_value_t<iterator_type>;
        using difference_type   = std::iter_difference_t<iterator_type>;
        using interator_concept = std::bidirectional_iterator_tag;

        constexpr const_iterator() noexcept = default;

        constexpr const_iterator(const slice_of* parent, iterator_type pos) noexcept
            : parent_(parent), pos_(pos) {}

        constexpr value_type operator*() const noexcept {
            value_type ret = *pos_;

            if (pos_ == parent_->begin_) {
                // this is the first buffer in the sequence so we need to exclude from its
                // beginning skip_front_ bytes
                ret += parent_->skip_front_;
            }

            // if pos_ is end_ then it's UB to dereference this iterator anyway
            BEMAN_SENDOSIO_CONTRACT_ASSERT(pos_ != parent_->end_);

            if (std::next(pos_) == parent_->end_) {
                // this is the last buffer in the sequence so we need to exclude from its
                // end skip_back_ bytes
                ret = make_buffer(ret, ret.size() - parent_->skip_back_);
            }

            return ret;
        }

        constexpr const_iterator& operator++() noexcept {
            ++pos_;
            return *this;
        }

        constexpr const_iterator operator++(int) noexcept {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        constexpr const_iterator& operator--() noexcept {
            --pos_;
            return *this;
        }

        constexpr const_iterator operator--(int) noexcept {
            auto tmp = *this;
            --(*this);
            return tmp;
        }

      private:
        const slice_of* parent_{};
        iterator_type   pos_{};

        constexpr friend bool operator==(const const_iterator& lhs,
                                         const const_iterator& rhs) noexcept {
            return lhs.pos_ == rhs.pos_;
        }
    };

    constexpr const_iterator begin() const noexcept { return {this, begin_}; }

    constexpr const_iterator end() const noexcept { return {this, end_}; }

  private:
    iterator_type begin_{};
    iterator_type end_{};
    // invariant:
    //    skip_front_ == 0 || (begin_ != end_ && skip_front_ < begin_->size())
    std::size_t skip_front_{};
    std::size_t skip_back_{};

    template <std::sentinel_for<iterator_type> Last>
    constexpr auto buffer_size_partial_sums(Last last) const noexcept {
        return buffer_sizes(std::ranges::subrange(begin_, last)) |
               std::views::transform(
                   // we set init to -skip_front_ to avoid subtracting skip_front_ from
                   // each partial sum. It's safe and correct because skip_front_ is
                   // unsigned, so the wrap-around is well-defined, and the first
                   // increment inside the transformation always undoes any wrapping
                   // because either:
                   //   1. we're initializing a sliced object and skip_front_ is zero, or
                   //   2. we've established the class invariants and skip_front_ is
                   //      strictly less than the size of the first buffer (unless there
                   //      is no first, in which case skip_front_ is both zero and
                   //      irrelevant).
                   [init = -skip_front_](std::size_t size) mutable noexcept {
                       init += size;

                       return init;
                   });
    }

    template <std::sentinel_for<iterator_type> Last>
    constexpr void initialize_back(Last last, std::size_t length) noexcept {
        // this is necessary because we're called after an invocation of update_front,
        // which has very likely changed begin_ from its initial value
        end_ = begin_;
        for (auto sum : buffer_size_partial_sums(last)) {
            ++end_;

            if (sum >= length) {
                // we found the earliest buffer containing enough cumulative bytes to
                // represent a total sequence of length length
                skip_back_ = sum - length;
                break;
            }
        }
    }

    template <std::ranges::view View>
    constexpr void advance_front(View buffers, std::size_t prefix) noexcept {
        for (auto size : buffer_sizes(buffers)) {
            if (size > prefix) {
                // we found the buffer we need to skip to; now adjust skip_front_ to skip
                // the right number of bytes into the new first buffer
                skip_front_ += prefix;
                break;
            }

            prefix -= size;
            skip_front_ = 0;
            ++begin_;
        }
    }

    constexpr void advance_front(std::size_t prefix) noexcept {
        advance_front(*this, prefix);
    }

    template <std::bidirectional_iterator First, std::sentinel_for<First> Last>
    constexpr void initialize_front(First first, Last last, std::size_t prefix) noexcept {
        advance_front(std::ranges::subrange(first, last), prefix);
    }

    template <const_buffer_sequence>
    friend struct beman::sendosio::consuming_buffers;
};

template <std::bidirectional_iterator Iterator>
struct slice_of<Iterator, true> : std::ranges::view_interface<slice_of<Iterator, true> > {
    using iterator_type = Iterator;

    using buffer_type = std::iter_value_t<iterator_type>;

    // the base template restricts its templated constructor to ensure it isn't stomping
    // on the copy constructor; this specialization doesn't need that because buffer_type
    // isn't slice_of
    static_assert(!std::same_as<slice_of, buffer_type>);

    using const_iterator = const buffer_type*;

    constexpr slice_of() noexcept = default;

    explicit constexpr slice_of(
        buffer_type seq,
        std::size_t offset = 0,
        std::size_t length = (std::numeric_limits<std::size_t>::max)()) noexcept
        : buffer_(make_buffer(seq + offset, length)) {}

    constexpr const_iterator begin() const noexcept {
        // empty() is either 0 or 1; when it's 0, we want begin() + 1 == end() and,
        // otherwise, we want begin() == end(). This achieve both, hopefully without a
        // branch.
        return std::addressof(buffer_) + empty();
    }

    constexpr const_iterator end() const noexcept { return std::addressof(buffer_) + 1; }

  private:
    buffer_type buffer_;

    constexpr bool empty() const noexcept { return buffer_.size() == 0; }

    constexpr void advance_front(std::size_t prefix) noexcept { buffer_ += prefix; }

    template <const_buffer_sequence>
    friend struct beman::sendosio::consuming_buffers;
};

template <const_buffer_sequence Buffers>
struct slice_type {
    using type =
        slice_of<decltype(sendosio::begin(std::declval<const Buffers&>())), false>;
};

template <std::convertible_to<const_buffer> Buffers>
struct slice_type<Buffers> {
    using type = slice_of<const Buffers*, true>;
};

template <class Iterator, bool Single>
struct slice_type<slice_of<Iterator, Single> > {
    using type = slice_of<Iterator, Single>;
};

} // namespace slice_of_detail

template <class Buffers>
using slice_type = slice_of_detail::slice_type<Buffers>::type;

} // namespace beman::sendosio

#endif // BEMAN_SENDOSIO_USE_MODULES() &&
       // !defined(BEMAN_SENDOSIO_INCLUDED_FROM_INTERFACE_UNIT)

#endif // BEMAN_SENDOSIO_DETAIL_BUFFERS_SLICE_OF_HPP
