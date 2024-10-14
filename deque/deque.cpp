#pragma once
#include <cstdint>
#include <exception>
#include <iterator>
#include <type_traits>

template <typename T>
class Deque {
  template <bool IsConst>
  class common_iterator;

 public:
  using iterator = common_iterator<false>;
  using const_iterator = common_iterator<true>;

  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  Deque()
      : arr_(nullptr),
        size_(0),
        origin_(nullptr),
        begin_(iterator()),
        end_(iterator()) {}

  Deque(size_t size)
      : arr_(new T*[3 * ((size + kNodeSize - 1) / kNodeSize)]),
        size_(size),
        origin_(new int64_t((3 * (size + kNodeSize - 1) / kNodeSize) / 2)),
        begin_(iterator(&arr_, origin_, nullptr,
                        (size + kNodeSize - 1) / kNodeSize - *origin_)),
        end_(
            iterator(&arr_, origin_, nullptr,
                     2 * ((size + kNodeSize - 1) / kNodeSize) - *origin_ - 1)) {
    set_nodes();
    begin_.ptr_ = arr_[*origin_ + begin_.c_node_];
    end_.ptr_ = arr_[*origin_ + end_.c_node_] + (size_ - 1) % kNodeSize;
    ++end_;
    set_elements();
  }

  Deque(size_t size, const T& value)
      : arr_(new T*[3 * ((size + kNodeSize - 1) / kNodeSize)]),
        size_(size),
        origin_(new int64_t((3 * (size + kNodeSize - 1) / kNodeSize) / 2)),
        begin_(iterator(&arr_, origin_, nullptr,
                        (size + kNodeSize - 1) / kNodeSize - *origin_)),
        end_(
            iterator(&arr_, origin_, nullptr,
                     2 * ((size + kNodeSize - 1) / kNodeSize) - *origin_ - 1)) {
    set_nodes();
    begin_.ptr_ = arr_[*origin_ + begin_.c_node_];
    end_.ptr_ = arr_[*origin_ + end_.c_node_] + (size_ - 1) % kNodeSize;
    ++end_;
    for (iterator iter = begin_; iter != end_; ++iter) {
      try {
        new (iter.ptr_) T(value);
      } catch (...) {
        destruction(iter);
        delete origin_;
        throw;
      }
    }
  }

  T& operator[](size_t ind) { return *(begin() + ind); }
  const T& operator[](size_t ind) const { return *(begin() + ind); }

  Deque(const Deque& deq)
      : arr_(new T*[3 * ((deq.size_ + kNodeSize - 1) / kNodeSize)]),
        size_(deq.size_),
        origin_(new int64_t((deq.size_ + kNodeSize - 1) / kNodeSize -
                            deq.begin_.c_node_)),
        begin_(iterator(&arr_, origin_, nullptr, deq.begin_.c_node_)),
        end_(iterator(&arr_, origin_, nullptr, deq.end_.c_node_)) {
    copying(deq);
  }

  Deque& operator=(const Deque& deq) {
    Deque copy(deq);
    std::swap(size_, copy.size_);
    std::swap(origin_, copy.origin_);
    begin_.iter_swap(copy.begin_);
    end_.iter_swap(copy.end_);
    std::swap(arr_, copy.arr_);
    if (begin_.iter_arr_ == nullptr) {
      begin_.iter_arr_ = &arr_;
      end_.iter_arr_ = &arr_;
    }
    return *this;
  }

  ~Deque() {
    destruction(end_);
    if (arr_ != nullptr) {
      delete origin_;
    }
  }

  size_t size() const { return size_; }
  bool empty() const { return size_ == 0; }

  T& at(size_t ind) {
    if (ind < size_) {
      return (*this)[ind];
    }
    std::out_of_range exception("out of range");
    throw exception;
  }

  const T& at(size_t ind) const {
    if (ind < size_) {
      return (*this)[ind];
    }
    std::out_of_range exception("out of range");
    throw exception;
  }

  void push_back(const T& value) {
    if (arr_ == nullptr) {
      Deque tmp(1, value);
      *this = tmp;
      return;
    }
    if (end_.c_node_ == *origin_) {
      allocation();
    }
    if (end_.ptr_ - arr_[*origin_ + end_.c_node_] == 0) {
      arr_[*origin_ + end_.c_node_] =
          reinterpret_cast<T*>(new uint8_t[kNodeSize * sizeof(T)]);
      end_.ptr_ = arr_[*origin_ + end_.c_node_];
    }
    try {
      new (end_.ptr_) T(value);
    } catch (...) {
      if (end_.ptr_ - arr_[*origin_ + end_.c_node_] == 0) {
        delete[] reinterpret_cast<uint8_t*>(arr_[*origin_ + end_.c_node_]);
        end_.ptr_ = arr_[*origin_ + end_.c_node_];
      }
      throw;
    }
    ++end_;
    ++size_;
  }

  void pop_back() {
    if (end_.ptr_ - arr_[*origin_ + end_.c_node_] == 1) {
      delete[] reinterpret_cast<uint8_t*>(arr_[*origin_ + end_.c_node_]);
    }
    --end_;
    end_.ptr_->~T();
    --size_;
  }

  void push_front(const T& value) {
    if (arr_ == nullptr) {
      Deque tmp(1, value);
      *this = tmp;
      return;
    }
    if (begin_.c_node_ == -*origin_) {
      allocation();
    }
    if (begin_.ptr_ - arr_[*origin_ + begin_.c_node_] == 0) {
      arr_[*origin_ + (--begin_.c_node_)] =
          reinterpret_cast<T*>(new uint8_t[kNodeSize * sizeof(T)]);
      begin_.ptr_ = arr_[*origin_ + begin_.c_node_] + kNodeSize - 1;
    } else {
      --begin_;
    }
    try {
      new (begin_.ptr_) T(value);
    } catch (...) {
      if (begin_.ptr_ - arr_[*origin_ + begin_.c_node_] == kNodeSize + 1) {
        delete[] reinterpret_cast<uint8_t*>(arr_[*origin_ + begin_.c_node_]);
        ++begin_;
        throw;
      }
    }
    ++size_;
  }

  void pop_front() {
    begin_.ptr_->~T();
    if (begin_.ptr_ - arr_[*origin_ + begin_.c_node_] == kNodeSize - 1) {
      delete[] reinterpret_cast<uint8_t*>(arr_[*origin_ + begin_.c_node_]);
    }
    ++begin_;
    --size_;
  }

  void insert(const iterator& iter, const T& value) {
    if (iter == end_) {
      push_back(value);
      return;
    }
    push_back(*(end_ - 1));
    for (iterator ite = end_ - 1; ite != iter; --ite) {
      try {
        ite->~T();
        new (ite.ptr_) T(*(ite - 1));
      } catch (...) {
        erase(ite);
        throw;
      }
    }
    try {
      iter->~T();
      new (iter.ptr_) T(value);
    } catch (...) {
      erase(iter);
      throw;
    }
  }

  void erase(const iterator& iter) {
    T value = *iter;
    for (iterator ite = iter; ite != end_ - 1; ++ite) {
      try {
        ite->~T();
        new (ite.ptr_) T(*(ite + 1));
      } catch (...) {
        --ite;
        for (; ite != iter; --ite) {
          ite->~T();
          new (ite.ptr_) T(*(ite - 1));
        }
        *ite = value;
        throw;
      }
    }
    pop_back();
  }

  iterator begin() { return begin_; }
  iterator end() { return end_; }

  const_iterator begin() const {
    iterator cbegin = begin_;
    return reinterpret_cast<const_iterator&>(cbegin);
  }
  const_iterator end() const {
    iterator cend = end_;
    return reinterpret_cast<const_iterator&>(cend);
  }

  const_iterator cbegin() const {
    iterator cbegin = begin_;
    return reinterpret_cast<const_iterator&>(cbegin);
  }
  const_iterator cend() const {
    iterator cend = end_;
    return reinterpret_cast<const_iterator&>(cend);
  }

  reverse_iterator rbegin() {
    iterator rbegin = end_;
    return std::make_reverse_iterator(rbegin);
  }
  reverse_iterator rend() {
    iterator rend = begin_;
    return std::make_reverse_iterator(rend);
  }

  const_reverse_iterator rbegin() const {
    iterator rbegin = end_;
    return std::make_reverse_iterator(
        reinterpret_cast<const_iterator&>(rbegin));
  }
  const_reverse_iterator rend() const {
    iterator rend = begin_;
    return std::make_reverse_iterator(reinterpret_cast<const_iterator&>(rend));
  }

  const_reverse_iterator crbegin() const {
    iterator crbegin = end_;
    return std::make_reverse_iterator(
        reinterpret_cast<const_iterator&>(crbegin));
  }
  const_reverse_iterator crend() const {
    iterator crend = begin_;
    return std::make_reverse_iterator(reinterpret_cast<const_iterator&>(crend));
  }

 private:
  T** arr_;
  size_t size_;
  int64_t* origin_;
  iterator begin_;
  iterator end_;
  static const size_t kAllocSize = 15;
  static const uint8_t kNodeSize = 64;

  void destruction(iterator up_to) {
    if (arr_ != nullptr && begin_.ptr_ != nullptr) {
      int64_t sgn = (end_.ptr_ - arr_[*origin_ + end_.c_node_] == 0 ? -1 : 0);
      for (iterator iter = begin_; iter != up_to; ++iter) {
        iter->~T();
      }
      for (int64_t ind = *origin_ + begin_.c_node_;
           ind <= *origin_ + end_.c_node_ + sgn; ++ind) {
        delete[] reinterpret_cast<uint8_t*>(arr_[ind]);
      }
    }
    delete[] arr_;
  }

  void allocation() {
    T** new_arr =
        new T*[kAllocSize * 3 * ((size_ + kNodeSize - 1) / kNodeSize)];
    int64_t new_origin = kAllocSize * ((size_ + kNodeSize - 1) / kNodeSize) +
                         ((size_ + kNodeSize - 1) / kNodeSize) - begin_.c_node_;
    for (int64_t ind = 0; ind <= end_.c_node_ - begin_.c_node_; ++ind) {
      new_arr[ind + new_origin + begin_.c_node_] =
          arr_[ind + *origin_ + begin_.c_node_];
      arr_[ind + *origin_ + begin_.c_node_] = nullptr;
    }
    delete[] arr_;
    arr_ = new_arr;
    (*origin_) = new_origin;
  }

  void copying(const Deque& deq) {
    if (deq.arr_ == nullptr) {
      return;
    }
    set_nodes();
    begin_.ptr_ = arr_[*origin_ + begin_.c_node_] +
                  (deq.begin_.ptr_ - (deq.arr_)[*origin_ + deq.begin_.c_node_]);
    end_.ptr_ = arr_[*origin_ + end_.c_node_] +
                (deq.end_.ptr_ - (deq.arr_)[*origin_ + deq.end_.c_node_]);
    size_t ind = 0;
    for (iterator iter = begin_; iter != end_; ++iter) {
      try {
        new (iter.ptr_) T(deq[ind++]);
      } catch (...) {
        destruction(iter);
        delete origin_;
      }
    }
  }

  void set_nodes() {
    for (int64_t ind = *origin_ + begin_.c_node_;
         ind <= *origin_ + end_.c_node_; ++ind) {
      try {
        arr_[ind] = reinterpret_cast<T*>(new uint8_t[kNodeSize * sizeof(T)]);
      } catch (...) {
        for (int64_t err_ind = *origin_ + begin_.c_node_; err_ind < ind;
             ++err_ind) {
          delete[] reinterpret_cast<uint8_t*>(arr_[ind]);
        }
        delete[] arr_;
        delete origin_;
        throw;
      }
    }
  }

  void set_elements() {
    for (iterator iter = begin_; iter != end_; ++iter) {
      try {
        new (iter.ptr_) T();
      } catch (...) {
        destruction(iter);
        delete origin_;
        throw;
      }
    }
  }
};

template <typename T>
template <bool IsConst>
class Deque<T>::common_iterator {
 public:
  using tp = std::conditional_t<IsConst, const T, T>;

  using difference_type = int64_t;
  using value_type = T;
  using pointer = tp*;
  using reference = tp&;
  using iterator_category = std::random_access_iterator_tag;

  operator common_iterator<false>() = delete;

  common_iterator()
      : iter_arr_(nullptr), origin_(nullptr), ptr_(nullptr), c_node_(0) {}

  common_iterator(T*** iter_arr, int64_t* origin, T* ptr, int64_t c_node)
      : iter_arr_(iter_arr), origin_(origin), ptr_(ptr), c_node_(c_node) {}

  common_iterator(const common_iterator& iter)
      : iter_arr_(iter.iter_arr_),
        origin_(iter.origin_),
        ptr_(iter.ptr_),
        c_node_(iter.c_node_) {}

  common_iterator& operator=(const common_iterator& iter) {
    iter_arr_ = iter.iter_arr_;
    origin_ = iter.origin_;
    ptr_ = iter.ptr_;
    c_node_ = iter.c_node_;
    return *this;
  }

  common_iterator& operator++() {
    if (ptr_ - (*iter_arr_)[*origin_ + c_node_] == kNodeSize - 1) {
      ptr_ = (*iter_arr_)[*origin_ + (++c_node_)];
    } else {
      ++ptr_;
    }
    return *this;
  }

  common_iterator operator++(int) {
    common_iterator copy = *this;
    ++(*this);
    return copy;
  }

  common_iterator& operator--() {
    if (ptr_ - (*iter_arr_)[*origin_ + c_node_] == 0) {
      ptr_ = (*iter_arr_)[*origin_ + (--c_node_)] + kNodeSize - 1;
    } else {
      --ptr_;
    }
    return *this;
  }

  common_iterator operator--(int) const {
    common_iterator copy = *this;
    --this;
    return copy;
  }

  common_iterator& operator+=(int64_t shift) {
    if (iter_arr_ != nullptr) {
      int64_t sgn = (shift >= 0 ? 1 : -1);
      int64_t sub_shift = ptr_ - (*iter_arr_)[*origin_ + c_node_] +
                          sgn * ((sgn * shift) % kNodeSize);
      c_node_ +=
          shift / kNodeSize + (sub_shift >= 0 ? sub_shift / kNodeSize : -1);
      ptr_ = sub_shift >= 0
                 ? (*iter_arr_)[*origin_ + c_node_] + sub_shift % kNodeSize
                 : (*iter_arr_)[*origin_ + c_node_] + kNodeSize + sub_shift;
    }
    return *this;
  }

  common_iterator& operator-=(int64_t shift) { return *this += (-shift); }

  common_iterator operator-(int64_t shift) const {
    common_iterator copy = *this;
    return copy -= shift;
  }

  common_iterator operator+(int64_t shift) const {
    common_iterator copy = *this;
    return copy += shift;
  }

  bool operator<(const common_iterator& iter) const {
    if (c_node_ < iter.c_node_) {
      return true;
    }
    if (c_node_ == iter.c_node_) {
      return iter.ptr_ - ptr_ > 0;
    }
    return false;
  }

  bool operator>(const common_iterator& iter) const { return iter < *this; }

  bool operator<=(const common_iterator& iter) const { return !(*this > iter); }

  bool operator>=(const common_iterator& iter) const { return !(*this < iter); }

  bool operator==(const common_iterator& iter) const {
    return *this <= iter && *this >= iter;
  }

  bool operator!=(const common_iterator& iter) const {
    return !(iter == *this);
  }

  int64_t operator-(const common_iterator& iter) const {
    if (c_node_ == iter.c_node_) {
      return ptr_ - iter.ptr_;
    }
    int64_t sgn = ((c_node_ - iter.c_node_) > 0 ? 1 : -1);
    int64_t dist_1 = ptr_ - (*iter_arr_)[*origin_ + c_node_] + 1;
    int64_t dist_2 = iter.ptr_ - (*iter_arr_)[*origin_ + iter.c_node_] + 1;
    return (c_node_ - iter.c_node_ - 1 * sgn) * kNodeSize +
           sgn * (sgn * (dist_1 - dist_2) + kNodeSize);
  }

  reference operator*() const { return *ptr_; }

  pointer operator->() const { return ptr_; }

  void iter_swap(iterator& iter) {
    std::swap(ptr_, iter.ptr_);
    std::swap(origin_, iter.origin_);
    std::swap(c_node_, iter.c_node_);
  }

 private:
  T*** iter_arr_;  // is a pointer to Deque, without such field iterator will be
                   // invalidated after allocation. if we would store T** then
                   // after allocation that pointer would be invalidated because
                   // we can't change that field in all iterators simultaneously

  int64_t* origin_;  // center of T** arr_
  T* ptr_;
  int64_t c_node_;  // with respect to origin_

  friend class Deque;
};
