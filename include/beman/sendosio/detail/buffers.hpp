// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_SENDOSIO_DETAIL_BUFFERS_HPP
#define BEMAN_SENDOSIO_DETAIL_BUFFERS_HPP

#include <beman/sendosio/config.hpp>

#if BEMAN_SENDOSIO_USE_MODULES() && !defined(BEMAN_SENDOSIO_INCLUDED_FROM_INTERFACE_UNIT)

import beman.sendosio;

#else

    #include <algorithm>
    #include <concepts>
    #include <cstddef>
    #include <ranges>
    #include <span>

namespace beman::sendosio {

struct mutable_buffer {
    constexpr mutable_buffer() noexcept = default;

    constexpr mutable_buffer(void* data, std::size_t size) noexcept
        : buffer_(static_cast<std::byte*>(data), size) {}

    constexpr void* data() const noexcept { return buffer_.data(); }

    constexpr std::size_t size() const noexcept { return buffer_.size(); }

    constexpr mutable_buffer& operator+=(std::size_t n) noexcept {
        n       = std::min(n, size());
        buffer_ = buffer_.last(size() - n);
        return *this;
    }

  private:
    std::span<std::byte> buffer_;
};

struct const_buffer {
    constexpr const_buffer() noexcept = default;

    constexpr const_buffer(const void* data, std::size_t size) noexcept
        : buffer_(static_cast<const std::byte*>(data), size) {}

    constexpr explicit(false) const_buffer(const mutable_buffer& other) noexcept
        : const_buffer(other.data(), other.size()) {}

    constexpr const void* data() const noexcept { return buffer_.data(); }

    constexpr std::size_t size() const noexcept { return buffer_.size(); }

    constexpr const_buffer& operator+=(std::size_t n) noexcept {
        n       = (std::min)(n, size());
        buffer_ = buffer_.last(size() - n);
        return *this;
    }

  private:
    std::span<const std::byte> buffer_;
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
    constexpr auto operator()(const Buffer& buffer) const noexcept {
        return (*this)(std::span<const Buffer, 1>(std::addressof(buffer)));
    }

    // TODO: is this overload necessary? I wonder if the compiler can deduce Buffers as a
    //       const T for some type T
    template <const_buffer_sequence Buffers>
        requires(!std::convertible_to<Buffers, const_buffer>)
    constexpr auto operator()(const Buffers& buffers) const noexcept {
        return CPO(buffers);
    }

    template <const_buffer_sequence Buffers>
        requires(!std::convertible_to<Buffers, const_buffer>)
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
        return std::ranges::fold_left(
            buffers | std::ranges::views::transform(std::ranges::size),
            std::size_t(0),
            std::plus<>{});
    }
};

inline constexpr buffer_size_t buffer_size{};

struct buffer_empty_t {
    template <const_buffer_sequence Buffers>
    constexpr bool operator()(const Buffers& buffers) const noexcept {
        return buffers | std::ranges::views::transform(std::ranges::size) |
               std::ranges::none_of([](auto size) noexcept { return size > 0; });
    }
};

inline constexpr buffer_empty_t buffer_empty{};

struct buffer_length_t {
    template <const_buffer_sequence Buffers>
    constexpr auto operator()(const Buffers& buffers) const noexcept {
        return std::ranges::distance(buffers);
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
