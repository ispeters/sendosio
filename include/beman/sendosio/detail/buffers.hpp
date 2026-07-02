// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_SENDOSIO_DETAIL_BUFFERS_HPP
#define BEMAN_SENDOSIO_DETAIL_BUFFERS_HPP

#include <beman/sendosio/config.hpp>

#include <beman/sendosio/detail/contracts.hpp>

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <ranges>
#include <span>

#if BEMAN_SENDOSIO_USE_MODULES() && !defined(BEMAN_SENDOSIO_INCLUDED_FROM_INTERFACE_UNIT)

import beman.sendosio;

#else

namespace beman::sendosio {

template <bool Const>
class basic_buffer {
  protected:
    using value_type = std::conditional_t<Const, const char, char>;
    using void_type  = std::conditional_t<Const, const void, void>;

    using pointer      = value_type*;
    using void_pointer = void_type*;

    std::span<value_type> buffer_;

  public:
    constexpr basic_buffer() noexcept = default;

    basic_buffer(void_pointer data, std::size_t size) noexcept
        : basic_buffer(static_cast<pointer*>(data), size) {}

    constexpr basic_buffer(pointer data, std::size_t size) noexcept
        BEMAN_SENDOSIO_PRE(data || (size == 0))
        : buffer_(data, size) {}

    basic_buffer(std::nullptr_t, std::size_t) = delete; // use the default ctor

    constexpr void_pointer data() const noexcept { return buffer_.data(); }

    constexpr std::size_t size() const noexcept { return buffer_.size(); }

    template <class Self, std::convertible_to<Self> Other>
    constexpr bool operator==(this Self lhs, Other rhs) noexcept {
        return lhs.data() == rhs.data() && lhs.size() == rhs.size();
    }

    template <class Self>
    constexpr Self& operator+=(this Self& self, std::size_t n) noexcept {
        self.buffer_ = self.buffer_.last(self.size() - (std::min)(n, self.size()));
        return self;
    }
};

struct const_buffer : private basic_buffer<true> {
    using base = basic_buffer<true>;

    friend base;

  public:
    using base::base;

    using base::data;
    using base::size;
    using base::operator+=;
    using base::operator==;
};

class mutable_buffer : private basic_buffer<false> {
    using base = basic_buffer<false>;

    friend base;

  public:
    using base::base;

    using base::data;
    using base::size;
    using base::operator+=;
    using base::operator==;

    constexpr operator const_buffer() const noexcept {
        // pass buffer_.data() instead of data() to make this constexpr
        return {buffer_.data(), size()};
    }
};

namespace detail {

template <class Buffers, class BufferType>
concept buffer_sequence =
    std::convertible_to<Buffers, BufferType> ||
    (std::ranges::bidirectional_range<Buffers> &&
     std::convertible_to<std::ranges::range_value_t<Buffers>, BufferType>);

}

template <class Buffers>
concept const_buffer_sequence = detail::buffer_sequence<Buffers, const_buffer>;

template <class Buffers>
concept mutable_buffer_sequence = detail::buffer_sequence<Buffers, mutable_buffer>;

namespace detail {

template <auto CPO>
struct begin_end_t {
    template <std::convertible_to<const_buffer> Buffer>
    constexpr auto operator()(Buffer& buffer) const noexcept {
        std::span<Buffer, 1> span(std::addressof(buffer), 1);
        return (*this)(span);
    }

    template <const_buffer_sequence Buffers>
        requires(!std::convertible_to<Buffers&, const_buffer>)
    constexpr auto operator()(Buffers& buffers) const noexcept {
        return CPO(buffers);
    }
};

} // namespace detail

inline constexpr detail::begin_end_t<std::ranges::begin> begin{};

inline constexpr detail::begin_end_t<std::ranges::end> end{};

struct buffer_size_t {
    template <const_buffer_sequence Buffers>
    constexpr std::size_t operator()(const Buffers& buffers) const noexcept {
        using namespace std::ranges;

        auto range = subrange(sendosio::begin(buffers), sendosio::end(buffers));

        return fold_left(range | views::transform(size), std::size_t(0), std::plus<>{});
    }
};

inline constexpr buffer_size_t buffer_size{};

struct buffer_empty_t {
    template <const_buffer_sequence Buffers>
    constexpr bool operator()(const Buffers& buffers) const noexcept {
        using namespace std::ranges;

        auto range = subrange(sendosio::begin(buffers), sendosio::end(buffers));

        return none_of(range | views::transform(size),
                       [](auto size) noexcept { return size > 0; });
    }
};

inline constexpr buffer_empty_t buffer_empty{};

struct buffer_length_t {
    template <const_buffer_sequence Buffers>
    constexpr auto operator()(const Buffers& buffers) const noexcept {
        using namespace std::ranges;

        auto range = subrange(sendosio::begin(buffers), sendosio::end(buffers));

        return distance(range);
    }
};

inline constexpr buffer_length_t buffer_length{};

template <const_buffer_sequence Buffer>
using buffer_type =
    std::conditional_t<mutable_buffer_sequence<Buffer>, mutable_buffer, const_buffer>;

} // namespace beman::sendosio

#endif // BEMAN_SENDOSIO_USE_MODULES() &&
       // !defined(BEMAN_SENDOSIO_INCLUDED_FROM_INTERFACE_UNIT)

#endif // BEMAN_SENDOSIO_TODO_HPP
