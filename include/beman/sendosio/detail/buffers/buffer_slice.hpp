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
                        std::size_t                      skip_first,
                        std::size_t                      skip_last,
                        std::iter_difference_t<Iterator> seq_length) noexcept
        : begin_(begin),
          end_(end),
          skip_first_(skip_first),
          skip_last_(skip_last),
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
                // beginning skip_first_ bytes
                ret += parent_->skip_first_;
            }

            if (index_ == (parent_->seq_length_ - 1)) {
                // this is the last buffer in the sequence so we need to exclude from its
                // end skip_last_ bytes
                ret = make_buffer(ret, ret.size() - parent_->skip_last_);
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

  private:
    Iterator                         begin_{};
    Iterator                         end_{};
    std::size_t                      skip_first_{};
    std::size_t                      skip_last_{};
    std::iter_difference_t<Iterator> seq_length_{};
};

template <class Buffers>
struct sliced {
    using iterator_type = decltype(sendosio::begin(std::declval<const Buffers&>()));
    using buffer_type   = sendosio::buffer_type<Buffers>;

  private:
    iterator_type begin_{};
    iterator_type end_{};

    // invariant: skip_front_ == 0 || skip_front_ < begin_->size()
    //
    // TODO: should the invariant be skip_front_ <= begin_->size()?
    //       I'm worried about the first buffer being empty
    std::size_t skip_front_{};
    std::size_t skip_back_{};

    std::iter_difference_t<iterator_type> seq_length_{};

    template <std::sentinel_for<iterator_type> Last>
    constexpr auto buffer_sizes(Last last) const noexcept {
        return std::ranges::subrange(begin_, last) |
               std::views::transform(std::ranges::size);
    }

    template <std::sentinel_for<iterator_type> Last>
    constexpr auto buffer_size_partial_sums(Last last) const noexcept {
        return buffer_sizes(last) |
               std::views::transform([init = std::size_t(0), adj = skip_front_](
                                         std::size_t size) mutable noexcept {
                   init += size;
                   // this follows from the class invariant
                   BEMAN_SENDOSIO_CONTRACT_ASSERT(init >= adj);
                   return init;
               });
    }

    template <std::sentinel_for<iterator_type> Last>
    constexpr void shrink_to_length(Last last, std::size_t length) noexcept {
        end_        = begin_;
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

    template <std::sentinel_for<iterator_type> Last>
    constexpr void remove_prefix(Last last, std::size_t prefix) noexcept {
        for (auto sum : buffer_size_partial_sums(last)) {
            if (sum > prefix) {
                // we found the buffer we need to skip to; now adjust skip_front_ to skip
                // the right number of bytes into the new first buffer
                BEMAN_SENDOSIO_CONTRACT_ASSERT(begin_->size() >= (sum - prefix));

                skip_front_ = begin_->size() - (sum - prefix);
                break;
            }

            ++begin_;
            --seq_length_;
        }
    }

  public:
    constexpr explicit sliced(const Buffers& seq,
                              std::size_t    offset,
                              std::size_t    length) noexcept
        : begin_(sendosio::begin(seq)) {
        // this initializes begin_ and skip_front_, and drives seq_length_ to a
        // meaningless negative value
        remove_prefix(sendosio::end(seq), offset);

        // this depends on begin_ and skip_front_ being initialized
        shrink_to_length(sendosio::end(seq), length);
    }

    constexpr data_view<iterator_type> data() const noexcept {
        return {begin_, end_, skip_front_, skip_back_, seq_length_};
    }

    constexpr void remove_prefix(std::size_t prefix) noexcept {
        remove_prefix(end_, prefix);
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
