// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_SENDOSIO_DETAIL_BUFFERS_MAKE_BUFFER_HPP
#define BEMAN_SENDOSIO_DETAIL_BUFFERS_MAKE_BUFFER_HPP

#include <beman/sendosio/config.hpp>

#if BEMAN_SENDOSIO_USE_MODULES() && !defined(BEMAN_SENDOSIO_INCLUDED_FROM_INTERFACE_UNIT)

import beman.sendosio;

#else

    #if !BEMAN_SENDOSIO_USE_MODULES()
        #include <beman/sendosio/detail/buffers.hpp>

        #include <concepts>
        #include <cstddef>
        #include <ranges>
        #include <string_view>
        #include <type_traits>
    #endif

namespace beman::sendosio {

namespace make_buffer_detail {

template <class T>
concept non_buffer_contiguous_range =          //
    std::ranges::contiguous_range<T>           //
    && std::ranges::sized_range<T>             //
    && !std::convertible_to<T, const_buffer>   //
    && !std::convertible_to<T, mutable_buffer> //
    && std::is_trivially_copyable_v<std::ranges::range_value_t<T> >;

template <class Buffer>
concept sendosio_buffer =
    std::same_as<Buffer, const_buffer> || std::same_as<Buffer, mutable_buffer>;

class make_buffer_t {
    template <class Buffer>
    static constexpr Buffer clamp(Buffer buffer, std::size_t max_size) noexcept {
        return normalize(Buffer::char_data(buffer), (std::min)(buffer.size(), max_size));
    }

    template <class Data>
    static constexpr auto normalize(Data* data, std::size_t size) noexcept
        -> std::conditional_t<std::is_const_v<Data>, const_buffer, mutable_buffer> {
        return {size ? data : nullptr, size};
    }

  public:
    template <sendosio_buffer Buffer>
    [[nodiscard]] constexpr Buffer operator()(Buffer buffer) const noexcept {
        return normalize(Buffer::char_data(buffer), buffer.size());
    }

    template <sendosio_buffer Buffer>
    [[nodiscard]] constexpr Buffer operator()(Buffer      buffer,
                                              std::size_t max_size) const noexcept {
        return clamp(buffer, max_size);
    }

    [[nodiscard]] constexpr mutable_buffer operator()(char*       data,
                                                      std::size_t size) const noexcept {
        return normalize(data, size);
    }

    [[nodiscard]] constexpr const_buffer operator()(const char* data,
                                                    std::size_t size) const noexcept {
        return normalize(data, size);
    }

    [[nodiscard]] mutable_buffer operator()(void* data, std::size_t size) const noexcept {
        return (*this)(static_cast<char*>(data), size);
    }

    [[nodiscard]] const_buffer operator()(const void* data,
                                          std::size_t size) const noexcept {
        return (*this)(static_cast<const char*>(data), size);
    }

    void operator()(std::nullptr_t, std::size_t) = delete;

    template <class Data>
        requires std::invocable<make_buffer_t, Data*, std::size_t>
    [[nodiscard]] constexpr auto
    operator()(Data* data, std::size_t size, std::size_t max_size) const noexcept {
        return clamp((*this)(data, size), max_size);
    }

    template <non_buffer_contiguous_range View>
        requires std::ranges::borrowed_range<View>
    [[nodiscard]] constexpr auto operator()(View&& view) const noexcept {
        using namespace std::ranges;

        return (*this)(data(view), size(view) * sizeof(range_value_t<View>));
    }

    template <non_buffer_contiguous_range View>
        requires std::ranges::borrowed_range<View>
    [[nodiscard]] constexpr auto operator()(View&&      view,
                                            std::size_t max_size) const noexcept {
        return clamp((*this)(std::forward<View>(view)), max_size);
    }

    template <non_buffer_contiguous_range Range>
    [[nodiscard]] constexpr auto operator()(Range& range) const noexcept {
        return (*this)(std::ranges::views::all(range));
    }

    template <non_buffer_contiguous_range Range>
    [[nodiscard]] constexpr auto operator()(Range&      range,
                                            std::size_t max_size) const noexcept {
        return clamp((*this)(range), max_size);
    }

    template <non_buffer_contiguous_range Range>
        requires(!std::ranges::borrowed_range<Range>)
    void operator()(const Range&&,
                    std::size_t = 0) const = delete; // avoid creating a dangling buffer
};

} // namespace make_buffer_detail

using make_buffer_detail::make_buffer_t;

inline constexpr make_buffer_t make_buffer{};

} // namespace beman::sendosio

#endif

#endif
