// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_SENDOSIO_DETAIL_BUFFERS_HPP
#define BEMAN_SENDOSIO_DETAIL_BUFFERS_HPP

#include <beman/sendosio/config.hpp>

#if BEMAN_SENDOSIO_USE_MODULES() && !defined(BEMAN_SENDOSIO_INCLUDED_FROM_INTERFACE_UNIT)

import beman.sendosio;

#else

    #include <beman/sendosio/detail/contracts.hpp>

    #include <algorithm>
    #include <concepts>
    #include <cstddef>
    #include <ranges>
    #include <span>

namespace beman::sendosio {

struct mutable_buffer {
    constexpr mutable_buffer() noexcept = default;

    mutable_buffer(void* data, std::size_t size) noexcept
        : mutable_buffer(static_cast<char*>(data), size) {}

    constexpr mutable_buffer(char* data, std::size_t size) noexcept
        BEMAN_SENDOSIO_PRE(data || (size == 0))
        : buffer_(data, size) {}

    mutable_buffer(std::nullptr_t, std::size_t) = delete; // use the default ctor

    constexpr void* data() const noexcept { return buffer_.data(); }

    constexpr std::size_t size() const noexcept { return buffer_.size(); }

    constexpr friend bool operator==(mutable_buffer lhs, mutable_buffer rhs) noexcept {
        return lhs.data() == rhs.data() && lhs.size() == rhs.size();
    }

    constexpr mutable_buffer& operator+=(std::size_t n) noexcept {
        buffer_ = buffer_.last(size() - (std::min)(n, size()));
        return *this;
    }

  private:
    friend struct const_buffer;

    std::span<char> buffer_;
};

struct const_buffer {
    constexpr const_buffer() noexcept = default;

    const_buffer(const void* data, std::size_t size) noexcept
        : const_buffer(static_cast<const char*>(data), size) {}

    constexpr const_buffer(const char* data, std::size_t size) noexcept
        BEMAN_SENDOSIO_PRE(data || (size == 0))
        : buffer_(data, size) {}

    const_buffer(std::nullptr_t, std::size_t) = delete; // use the default ctor

    constexpr explicit(false) const_buffer(const mutable_buffer& other) noexcept
        : const_buffer(other.buffer_.data(), other.size()) {}

    constexpr const void* data() const noexcept { return buffer_.data(); }

    constexpr std::size_t size() const noexcept { return buffer_.size(); }

    constexpr friend bool operator==(const_buffer lhs, const_buffer rhs) noexcept {
        return lhs.data() == rhs.data() && lhs.size() == rhs.size();
    }

    constexpr const_buffer& operator+=(std::size_t n) noexcept {
        buffer_ = buffer_.last(size() - (std::min)(n, size()));
        return *this;
    }

  private:
    std::span<const char> buffer_;
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
