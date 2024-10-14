#pragma once

#include <cstddef>

class RingBuffer {
 public:
  explicit RingBuffer(size_t capacity)
      : size_of_ring_(capacity), Ring_(new int[capacity]) {}

  size_t Size() const { return number_of_elements_; }

  bool Empty() const { return number_of_elements_ == 0; }

  bool TryPush(int element) {
    if (number_of_elements_ == size_of_ring_) {
      return false;
    }
    Ring_[index_end_] = element;
    ++index_end_ %= size_of_ring_;
    ++number_of_elements_;
    return true;
  }

  bool TryPop(int* element) {
    if (this->Empty()) {
      return false;
    }
    *element = Ring_[index_begin_];
    ++index_begin_ %= size_of_ring_;
    --number_of_elements_;
    return true;
  }

  ~RingBuffer() { delete[] Ring_; }

 private:
  size_t number_of_elements_ = 0;
  size_t size_of_ring_ = 0;
  int* Ring_ = nullptr;
  size_t index_end_ = 0;
  size_t index_begin_ = 0;
};
