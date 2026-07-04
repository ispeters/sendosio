// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_SENDOSIO_DETAIL_BUFFERS_HPP
#define BEMAN_SENDOSIO_DETAIL_BUFFERS_HPP

#include <beman/sendosio/config.hpp>

#if BEMAN_SENDOSIO_USE_MODULES() && !defined(BEMAN_SENDOSIO_INCLUDED_FROM_INTERFACE_UNIT)

import beman.sendosio;

#else

    #if !BEMAN_SENDOSIO_USE_MODULES()
        #include <beman/sendosio/detail/contracts.hpp>

        #include <algorithm>
        #include <concepts>
        #include <cstddef>
        #include <ranges>
    #endif

namespace beman::sendosio {

template <bool Const>
class basic_buffer {
  protected:
    using value_type = std::conditional_t<Const, const char, char>;
    using void_type  = std::conditional_t<Const, const void, void>;

    using pointer      = value_type*;
    using void_pointer = void_type*;

    using size_type = std::size_t;

    pointer   data_{};
    size_type size_{};

    template <class Buffer>
        requires std::is_base_of_v<basic_buffer, Buffer>
    friend constexpr Buffer& operator+=(Buffer& buffer, size_type n) noexcept {
        n = (std::min)(n, buffer.size());

        // the explicit cast to base is required for MSVC to permit us to access data_;
        // the other supported compilers don't need the case because the derived classes
        // declare us as a friend but that's not enough for MSVC. Mysteriously, a
        // static_cast is insufficient--only a C-style cast overrides the access modifier
        buffer = Buffer(((basic_buffer&)buffer).data_ + n, buffer.size() - n);

        return buffer;
    }

    friend constexpr bool operator==(basic_buffer, basic_buffer) noexcept = default;

  public:
    constexpr basic_buffer() noexcept = default;

    basic_buffer(void_pointer data, size_type size) noexcept
        : basic_buffer(static_cast<pointer*>(data), size) {}

    constexpr basic_buffer(pointer data, size_type size) noexcept
        BEMAN_SENDOSIO_PRE(data || (size == 0))
        : data_(data), size_(size) {}

    basic_buffer(std::nullptr_t, size_type) = delete; // use the default ctor

    constexpr void_pointer data() const noexcept { return data_; }

    constexpr size_type size() const noexcept { return size_; }
};

class const_buffer : basic_buffer<true> {
    using base = basic_buffer<true>;

    friend base;

    constexpr friend bool operator==(const_buffer, const_buffer) noexcept = default;

  public:
    using base::base;

    using base::data;
    using base::size;
};

class mutable_buffer : basic_buffer<false> {
    using base = basic_buffer<false>;

    friend base;

    constexpr friend bool operator==(mutable_buffer, mutable_buffer) noexcept = default;

  public:
    using base::base;

    using base::data;
    using base::size;

    constexpr operator const_buffer() const noexcept {
        // pass data_ instead of data() to make this constexpr
        return {data_, size()};
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
        auto address = std::addressof(buffer);
        auto range   = std::ranges::subrange(address, address + 1);

        return (*this)(range);
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
