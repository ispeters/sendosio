// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_SENDOSIO_DETAIL_BUFFERS_HPP
#define BEMAN_SENDOSIO_DETAIL_BUFFERS_HPP

#include <beman/sendosio/config.hpp>

#if BEMAN_SENDOSIO_USE_MODULES() && !defined(BEMAN_SENDOSIO_INCLUDED_FROM_INTERFACE_UNIT)

import beman.sendosio;

#else

    #include <algorithm>
    #include <cstddef>
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

} // namespace beman::sendosio

#endif // BEMAN_SENDOSIO_USE_MODULES() &&
       // !defined(BEMAN_SENDOSIO_INCLUDED_FROM_INTERFACE_UNIT)

#endif // BEMAN_SENDOSIO_TODO_HPP
