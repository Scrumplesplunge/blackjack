#pragma once

#include <cassert>
#include <cstddef>
#include <iterator>

namespace nonstd {

template <typename T>
class span {
 public:
  using size_type = std::ptrdiff_t;
  using index_type = size_type;
  using iterator = T*;
  using const_iterator = const T*;

  // Construct an empty span.
  constexpr span() noexcept
      : data_(nullptr), size_(0) {}
  // Construct a span from a position and a size.
  constexpr span(T* data, size_type size) noexcept
      : data_(data), size_(size) {}
  // Construct a span from a pointer range.
  constexpr span(T* first, T* last) noexcept
      : data_(first), size_(last - first) {}
  // Construct a span from a contiguous container.
  template <typename Container>
  constexpr span(Container& container) noexcept
      : span(std::data(container), std::size(container)) {}
  template <typename Container>
  constexpr span(const Container& container) noexcept
      : span(std::data(container), std::size(container)) {}
  // Copy another span.
  constexpr span(const span<T>& other) noexcept = default;

  // Access the contents.
  constexpr iterator begin() const noexcept { return data_; }
  constexpr iterator end() const noexcept { return data_ + size_; }
  constexpr T& operator[](index_type index) const {
    assert(0 <= index && index < size_);
    return data_[index];
  }
  constexpr T* data() const noexcept { return data_; }
  constexpr size_type size() const noexcept { return size_; }
  constexpr bool empty() const noexcept { return size_ == 0; }

  // Create a subspan of the first n elements.
  constexpr span first(size_type n) const noexcept {
    assert(0 <= n && n <= size_);
    return span{data_, n};
  }

  // Create a subspan of the last n elements.
  constexpr span last(size_type n) const noexcept {
    assert(0 <= n && n <= size_);
    return span{data_ + size_ - n, n};
  }

  // Create a subspan of the count elements starting at offset.
  constexpr span subspan(size_type offset, size_type count) const noexcept {
    assert(0 <= offset && offset <= size_);
    assert(0 <= count && offset + count <= size_);
    return span{data_ + offset, count};
  }

 private:
  T* data_;
  size_type size_;
};

}  // namespace nonstd
