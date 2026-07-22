// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/sendosio/config.hpp>

#include <catch2/catch_all.hpp>

#include <exec/get_frame_allocator.hpp>

#include <beman/sendosio/detail/buffers.hpp>
#include <beman/sendosio/detail/buffers/buffer_copy.hpp>
#include <beman/sendosio/detail/buffers/make_buffer.hpp>
#include <beman/sendosio/detail/read.hpp>
#include <beman/sendosio/detail/vendor/execution.hpp>
#include <stdexec/execution.hpp>

#include <array>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>

namespace {

namespace sendosio = beman::sendosio;
namespace ex       = sendosio::ex;

enum class io_errc {
    eof = 1,
};

class io_category_t : public std::error_category {
  public:
    const char* name() const noexcept override { return "io"; }
    std::string message(int ev) const override {
        switch (static_cast<io_errc>(ev)) {
        case io_errc::eof:
            return "end of file";
        default:
            return "unknown io error";
        }
    }
};

const std::error_category& io_category() {
    static io_category_t instance;
    return instance;
}

std::error_code make_error_code(io_errc e) {
    return {static_cast<int>(e), io_category()};
}

} // namespace

// Enables implicit conversion to std::error_code / comparison with it
template <>
struct std::is_error_code_enum<io_errc> : std::true_type {};

namespace {

struct read_source {
    explicit read_source(
        std::size_t max_read_size = (std::numeric_limits<std::size_t>::max)()) noexcept
        : max_read_size_(max_read_size) {}

    void provide(std::string_view sv) { data_.append(sv); }

    void clear() noexcept {
        data_.clear();
        pos_ = 0;
    }

    std::size_t available() noexcept { return data_.size() - pos_; }

    template <sendosio::mutable_buffer_sequence Buffers>
    ex::sender auto read_some(Buffers buffers) {
        return ex::just() |
               ex::let_value([this, buffers = std::move(buffers)]() noexcept {
                   if (sendosio::buffer_empty(buffers)) {
                       return ex::just(std::error_code(), std::size_t());
                   }

                   if (pos_ >= data_.size()) {
                       return ex::just(make_error_code(io_errc::eof), std::size_t());
                   }

                   std::size_t avail = (std::min)(max_read_size_, data_.size() - pos_);
                   auto        src   = sendosio::make_buffer(data_.data() + pos_, avail);

                   const auto n = sendosio::buffer_copy(buffers, src);
                   pos_ += n;
                   return ex::just(std::error_code(), n);
               });
    }

  private:
    std::string data_;
    std::size_t pos_ = 0;
    std::size_t max_read_size_;
};

TEST_CASE("read returns a sender", "[sendosio::read]") {
    constexpr std::string_view text("hello, world!");

    read_source stream;
    stream.provide(text);

    std::array<char, 20> buffer;

    auto sndr = sendosio::read(stream, sendosio::make_buffer(buffer));
    STATIC_REQUIRE(ex::sender<decltype(sndr)>);

    auto [ec, n] = ex::sync_wait(std::move(sndr)).value();

    REQUIRE(ec == io_errc::eof);
    REQUIRE(n == text.size());

    REQUIRE(std::string_view(buffer.data(), n) == text);
}

} // namespace
