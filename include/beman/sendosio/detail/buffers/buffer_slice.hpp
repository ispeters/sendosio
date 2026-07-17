// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_SENDOSIO_DETAIL_BUFFERS_BUFFER_SLICE_HPP
#define BEMAN_SENDOSIO_DETAIL_BUFFERS_BUFFER_SLICE_HPP

#include <beman/sendosio/config.hpp>

#if BEMAN_SENDOSIO_USE_MODULES() && !defined(BEMAN_SENDOSIO_INCLUDED_FROM_INTERFACE_UNIT)

import beman.sendosio;

#else

    #if !BEMAN_SENDOSIO_USE_MODULES()
        #include <beman/sendosio/detail/buffers.hpp>
        #include <beman/sendosio/detail/buffers/make_buffer.hpp>
        #include <beman/sendosio/detail/concept/slice.hpp>
        #include <beman/sendosio/detail/contracts.hpp>

        #include <cstddef>
        #include <iterator>
        #include <limits>
        #include <ranges>
        #include <utility>
    #endif

namespace beman::sendosio {
namespace buffer_slice_detail {

// heavily inspired by Claude Sonnet 5
// https://claude.ai/share/cb3523ee-45b9-4984-b6af-2c0da373206a
template <class Iterator>
struct data_view : std::ranges::view_interface<data_view<Iterator> > {
    constexpr data_view() noexcept = default;

    constexpr data_view(Iterator                         begin,
                        Iterator                         end,
                        std::size_t                      skip_front,
                        std::size_t                      skip_back,
                        std::iter_difference_t<Iterator> seq_length) noexcept
        : begin_(begin),
          end_(end),
          skip_front_(skip_front),
          skip_back_(skip_back),
          seq_length_(seq_length) {}

    struct const_iterator {
        using value_type        = std::iter_value_t<Iterator>;
        using difference_type   = std::iter_difference_t<Iterator>;
        using interator_concept = std::bidirectional_iterator_tag;

        constexpr const_iterator() noexcept = default;

        constexpr const_iterator(const data_view* parent,
                                 Iterator         pos,
                                 difference_type  index) noexcept
            : parent_(parent), pos_(pos), index_(index) {}

        constexpr value_type operator*() const noexcept {
            value_type ret = *pos_;

            if (index_ == 0) {
                // this is the first buffer in the sequence so we need to exclude from its
                // beginning skip_front_ bytes
                ret += parent_->skip_front_;
            }

            if (index_ == (parent_->seq_length_ - 1)) {
                // this is the last buffer in the sequence so we need to exclude from its
                // end skip_back_ bytes
                ret = make_buffer(ret, ret.size() - parent_->skip_back_);
            }

            return ret;
        }

        constexpr const_iterator& operator++() noexcept {
            ++pos_;
            ++index_;
            return *this;
        }

        constexpr const_iterator operator++(int) noexcept {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        constexpr const_iterator& operator--() noexcept {
            --pos_;
            --index_;
            return *this;
        }

        constexpr const_iterator operator--(int) noexcept {
            auto tmp = *this;
            --(*this);
            return tmp;
        }

      private:
        const data_view* parent_{};
        Iterator         pos_{};
        difference_type  index_{0};

        constexpr friend bool operator==(const const_iterator& lhs,
                                         const const_iterator& rhs) noexcept {
            return lhs.pos_ == rhs.pos_;
        }
    };

    constexpr const_iterator begin() const noexcept { return {this, begin_, 0}; }

    constexpr const_iterator end() const noexcept { return {this, end_, seq_length_}; }

    constexpr std::iter_difference_t<Iterator> size() const noexcept {
        return seq_length_;
    }

  private:
    Iterator                         begin_{};
    Iterator                         end_{};
    std::size_t                      skip_front_{};
    std::size_t                      skip_back_{};
    std::iter_difference_t<Iterator> seq_length_{};
};

template <class Buffers>
struct sliced {
    using iterator_type = decltype(sendosio::begin(std::declval<const Buffers&>()));
    using buffer_type   = sendosio::buffer_type<Buffers>;

  private:
    iterator_type begin_{};
    iterator_type end_{};

    // invariant:
    //    skip_front_ == 0 || (begin_ != end_ && skip_front_ < begin_->size())
    std::size_t skip_front_{};
    std::size_t skip_back_{};

    std::iter_difference_t<iterator_type> seq_length_{};

    template <std::ranges::view View>
    constexpr auto buffer_sizes(View buffers) const noexcept {
        return buffers | std::views::transform(std::ranges::size);
    }

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
        if constexpr (std::random_access_iterator<iterator_type>) {
            // if iterator_type is random-access then this contract check is cheap
            BEMAN_SENDOSIO_CONTRACT_ASSERT(
                // update_front may have advanced begin_ (leaving end_ alone); for each
                // such advancement, it has decremented seq_length_
                end_ <= begin_ && seq_length_ == -std::ranges::distance(end_, begin_));
        }

        // this is necessary because we're called after an invocation of update_front,
        // which has very likely changed begin_ from its initial value
        end_ = begin_;
        // this is also necessary because update_front decrements seq_length_ if it
        // advances begin_, so it may be negative
        seq_length_ = 0;

        for (auto sum : buffer_size_partial_sums(last)) {
            ++end_;
            ++seq_length_;

            if (sum >= length) {
                // we found the earliest buffer containing enough cumulative bytes to
                // represent a total sequence of length length
                skip_back_ = sum - length;
                break;
            }
        }
    }

    template <std::ranges::view View>
    constexpr void update_front(View buffers, std::size_t prefix) noexcept {
        for (auto size : buffer_sizes(buffers)) {
            if (size > prefix) {
                // we found the buffer we need to skip to; now adjust skip_front_ to skip
                // the right number of bytes into the new first buffer
                skip_front_ = prefix;
                break;
            }

            prefix -= size;
            ++begin_;
            --seq_length_;
        }
    }

    template <std::bidirectional_iterator First, std::sentinel_for<First> Last>
    constexpr void initialize_front(First first, Last last, std::size_t prefix) noexcept {
        update_front(std::ranges::subrange(first, last), prefix);
    }

  public:
    constexpr explicit sliced(const Buffers& seq,
                              std::size_t    offset,
                              std::size_t    length) noexcept
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

    constexpr data_view<iterator_type> data() const noexcept {
        return {begin_, end_, skip_front_, skip_back_, seq_length_};
    }

    constexpr void remove_prefix(std::size_t prefix) noexcept {
        // update begin_, skip_front_, and seq_length_ to account for having removed
        // prefix bytes from the front of the buffer sequence
        update_front(data(), prefix);
    }
};

template <class Buffers>
sliced(const Buffers&) -> sliced<Buffers>;

struct buffer_slice_t {
    template <class Buffers>
        requires mutable_buffer_sequence<Buffers> || const_buffer_sequence<Buffers>
    constexpr slice auto operator()(
        const Buffers& seq,
        std::size_t    offset = 0,
        std::size_t length = (std::numeric_limits<std::size_t>::max)()) const noexcept {
        return sliced(seq, offset, length);
    }

    // Capy deletes this overload to avoid creating dangling slices; I think it might be
    // too restrictive because rvalue views are usually safe, but I'll need tests to
    // confirm
    template <class Buffers>
        requires mutable_buffer_sequence<Buffers> || const_buffer_sequence<Buffers>
    void operator()(const Buffers&&, std::size_t = 0, std::size_t = 0) const = delete;
};

} // namespace buffer_slice_detail

using buffer_slice_detail::buffer_slice_t;

inline constexpr buffer_slice_t buffer_slice{};

} // namespace beman::sendosio

#endif // BEMAN_SENDOSIO_USE_MODULES() &&
       // !defined(BEMAN_SENDOSIO_INCLUDED_FROM_INTERFACE_UNIT)

#endif // BEMAN_SENDOSIO_DETAIL_BUFFERS_BUFFER_SLICE_HPP
