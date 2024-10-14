#pragma once
#include <initializer_list>
#include <iterator>
#include <memory>
#include <type_traits>
#include <vector>

template <typename T, typename Allocator = std::allocator<T>>
class Deque {
  template <bool IsConst>
  class base_iterator;

  using alloc = std::allocator_traits<Allocator>;

 public:
  using value_type = T;

  using allocator_type = Allocator;

  using iterator = base_iterator<false>;
  using const_iterator = base_iterator<true>;

  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

 private:
  size_t size_;
  std::vector<T*> storage_;
  int64_t origin_;
  iterator begin_;
  iterator end_;
  [[no_unique_address]] allocator_type alloc_;
  static const int64_t kNodeSize = 32;
  static const size_t kAllocSize = 45;

  void destroy(const iterator& up_to) {
    if (size_ != 0) {
      int64_t sgn = (storage_[end_.c_node_ + origin_] == end_.ptr_ ? -1 : 0);
      for (iterator iter = begin_; iter < up_to; ++iter) {
        alloc::destroy(alloc_, iter.ptr_);
      }
      for (int64_t ind = begin_.c_node_; ind <= end_.c_node_ + sgn; ++ind) {
        alloc::deallocate(alloc_, storage_[ind + origin_], kNodeSize);
        storage_[ind + origin_] = nullptr;
      }
    }
  }

  void increment() const noexcept {
  }  // for increment() in make_deque(args...) with Args = {}

  template <typename U>
  decltype(auto) increment(U&& value) {
    if constexpr (std::is_same_v<std::remove_cvref_t<U>, T>) {
      return std::forward<U>(value);
    } else {
      return *(value++);
    }
  }

  template <typename U>
  U&& my_forward(std::remove_reference_t<U>& value) {
    return static_cast<U&&>(value);
  }

  void my_forward() const noexcept {
  }  // for my_forward() in make_deque(args...) with Args = {}

  void make_frame() {
    for (size_t ind = storage_.size() / 3; ind <= 2 * (storage_.size() / 3) - 1;
         ++ind) {
      storage_[ind] = alloc::allocate(alloc_, kNodeSize);
    }
    begin_.ptr_ = storage_[begin_.c_node_ + origin_];
    end_.ptr_ = storage_[end_.c_node_ + origin_] + (size_ - 1) % kNodeSize;
    ++end_;
  }

  template <typename... Args>
  void make_deque(Args&&... args) {
    make_frame();
    for (iterator iter = begin_; iter < end_; ++iter) {
      try {
        alloc::construct(alloc_, iter.ptr_,
                         increment(my_forward<Args>(args))...);
      } catch (...) {
        destroy(iter);
        throw;
      }
    }
  }

  void allocation() {
    std::vector<T*> new_storage(((size_ + kNodeSize - 1) / kNodeSize) *
                                kAllocSize);
    int64_t new_origin =
        (new_storage.size() - begin_.c_node_ - end_.c_node_ - 4) / 2;
    for (int64_t ind = begin_.c_node_; ind <= end_.c_node_; ++ind) {
      new_storage[new_origin + ind] = storage_[origin_ + ind];
    }
    storage_ = std::move(new_storage);
    origin_ = new_origin;
  }

  void deque_swap(Deque& other) {
    std::swap(size_, other.size_);
    std::swap(origin_, other.origin_);
    std::swap(storage_, other.storage_);
    begin_.iter_swap(other.begin_);
    end_.iter_swap(other.end_);
    begin_.storage_ = &storage_;
    end_.storage_ = &storage_;
    begin_.origin_ = &origin_;
    end_.origin_ = &origin_;
  }

  template <typename... Args>
  void push_construct(Args&&... args) {
    Deque tmp(1, T(std::forward<Args>(args)...));
    alloc::destroy(alloc_, tmp.begin_.ptr_);
    *this = std::move(tmp);
    try {
      alloc::construct(alloc_, begin_.ptr_, std::forward<Args>(args)...);
    } catch (...) {
      destroy(end_);
      throw;
    }
  }

  template <typename... Args>
  void safety_construct_front(Args&&... args) {
    try {
      alloc::construct(alloc_, begin_.ptr_, std::forward<Args>(args)...);
    } catch (...) {
      ++begin_;
      if (begin_.ptr_ == storage_[origin_ + begin_.c_node_]) {
        alloc::deallocate(alloc_, storage_[origin_ + begin_.c_node_ - 1],
                          kNodeSize);
        storage_[origin_ + begin_.c_node_ - 1] = nullptr;
      }
      throw;
    }
  }

  template <typename... Args>
  void safety_construct_back(Args&&... value) {
    try {
      alloc::construct(alloc_, end_.ptr_, std::forward<Args>(value)...);
    } catch (...) {
      if (end_.ptr_ == storage_[origin_ + end_.c_node_]) {
        alloc::deallocate(alloc_, storage_[origin_ + end_.c_node_], kNodeSize);
        end_.ptr_ = storage_[origin_ + end_.c_node_] = nullptr;
      }
      throw;
    }
  }

 public:
  Deque() : size_(0), origin_(0) {}

  Deque(const Allocator& alloc) : size_(0), origin_(0), alloc_(alloc) {}

  Deque(size_t size, const Allocator& alloc = Allocator())
      : size_(size),
        storage_(3 * ((size_ + kNodeSize - 1) / kNodeSize)),
        origin_(storage_.size() / 2),
        begin_(&storage_, nullptr, &origin_, storage_.size() / 3 - origin_),
        end_(&storage_, nullptr, &origin_,
             2 * (storage_.size() / 3) - 1 - origin_),
        alloc_(alloc) {
    if (size_ != 0) {
      make_deque();
    }
  }

  Deque(size_t size, const T& value, const Allocator& alloc = Allocator())
      : size_(size),
        storage_(3 * ((size_ + kNodeSize - 1) / kNodeSize)),
        origin_(storage_.size() / 2),
        begin_(&storage_, nullptr, &origin_, storage_.size() / 3 - origin_),
        end_(&storage_, nullptr, &origin_,
             2 * (storage_.size() / 3) - 1 - origin_),
        alloc_(alloc) {
    if (size_ != 0) {
      make_deque(value);
    }
  }

  Deque(size_t size, T&& value, const Allocator& alloc = Allocator())
      : size_(size),
        storage_(3 * ((size_ + kNodeSize - 1) / kNodeSize)),
        origin_(storage_.size() / 2),
        begin_(&storage_, nullptr, &origin_, storage_.size() / 3 - origin_),
        end_(&storage_, nullptr, &origin_,
             2 * (storage_.size() / 3) - 1 - origin_),
        alloc_(alloc) {
    if (size_ != 0) {
      make_deque(std::move(value));
    }
  }

  Deque(const Deque& other)
      : size_(other.size_),
        storage_(3 * ((size_ + kNodeSize - 1) / kNodeSize)),
        origin_(storage_.size() / 2),
        begin_(&storage_, nullptr, &origin_, storage_.size() / 3 - origin_),
        end_(&storage_, nullptr, &origin_,
             2 * (storage_.size() / 3) - 1 - origin_),
        alloc_(alloc::select_on_container_copy_construction(other.alloc_)) {
    if (size_ != 0) {
      auto value = other.begin();
      make_deque(value);
    }
  }

  Deque(const std::initializer_list<T>& other,
        const Allocator& alloc = Allocator())
      : size_(other.size()),
        storage_(3 * ((size_ + kNodeSize - 1) / kNodeSize)),
        origin_(storage_.size() / 2),
        begin_(&storage_, nullptr, &origin_, storage_.size() / 3 - origin_),
        end_(&storage_, nullptr, &origin_,
             2 * (storage_.size() / 3) - 1 - origin_),
        alloc_(alloc) {
    if (size_ != 0) {
      auto value = other.begin();
      make_deque(value);
    }
  }

  Deque(Deque&& other) : Deque() { deque_swap(other); }

  ~Deque() { destroy(end_); }

  Deque& operator=(const Deque& other) {
    if constexpr (alloc::propagate_on_container_copy_assignment::value) {
      if (get_allocator() != other.get_allocator()) {
        destroy(end_);
      }
      alloc_ = other.alloc_;
    }
    Deque copy(other);
    deque_swap(copy);
    return *this;
  }

  Deque& operator=(Deque&& other) {
    if constexpr (alloc::propagate_on_container_move_assignment::value) {
      if (get_allocator() != other.get_allocator()) {
        destroy(end_);
      }
      alloc_ = other.alloc_;
    }
    Deque copy(std::move(other));
    deque_swap(copy);
    return *this;
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

  T& operator[](size_t ind) { return *(begin_ + ind); }
  const T& operator[](size_t ind) const { return *(begin_ + ind); }

  void push_back(const T& value) { emplace_back(value); }

  void push_back(T&& value) { emplace_back(std::move(value)); }

  void push_front(const T& value) { emplace_front(value); }

  void push_front(T&& value) { emplace_front(std::move(value)); }

  void pop_back() {
    if (end_.ptr_ - storage_[origin_ + end_.c_node_] == 1) {
      alloc::deallocate(alloc_, storage_[origin_ + end_.c_node_], kNodeSize);
    }
    --end_;
    alloc::destroy(alloc_, end_.ptr_);
    --size_;
  }

  void pop_front() {
    alloc::destroy(alloc_, begin_.ptr_);
    ++begin_;
    if (begin_.ptr_ == storage_[origin_ + begin_.c_node_]) {
      alloc::deallocate(alloc_, storage_[origin_ + begin_.c_node_ - 1],
                        kNodeSize);
      storage_[origin_ + begin_.c_node_ - 1] = nullptr;
    }
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
        alloc::destroy(alloc_, ite.ptr_);
        alloc::construct(alloc_, ite.ptr_, *(ite - 1));
      } catch (...) {
        erase(ite);
        throw;
      }
    }
    try {
      alloc::destroy(alloc_, iter.ptr_);
      alloc::construct(alloc_, iter.ptr_, value);
    } catch (...) {
      erase(iter);
      throw;
    }
  }

  void erase(const iterator& iter) {
    T value = *iter;
    for (iterator ite = iter; ite != end_ - 1; ++ite) {
      try {
        alloc::destroy(alloc_, ite.ptr_);
        alloc::construct(alloc_, ite.ptr_, *(ite + 1));
      } catch (...) {
        --ite;
        for (; ite != iter; --ite) {
          alloc::destroy(alloc_, ite.ptr_);
          alloc::construct(alloc_, ite.ptr_, *(ite - 1));
        }
        *ite = value;
        throw;
      }
    }
    pop_back();
  }

  template <typename... Args>
  void emplace_back(Args&&... args) {
    if (size_ == 0) {
      push_construct(std::forward<Args>(args)...);
      return;
    }
    if (end_.c_node_ + origin_ == static_cast<int64_t>(storage_.size()) - 1) {
      allocation();
    }
    if (end_.ptr_ == storage_[origin_ + end_.c_node_]) {
      storage_[origin_ + end_.c_node_] = alloc::allocate(alloc_, kNodeSize);
      end_.ptr_ = storage_[origin_ + end_.c_node_];
    }
    safety_construct_back(std::forward<Args>(args)...);
    ++end_;
    ++size_;
  }

  template <typename... Args>
  void emplace_front(Args&&... args) {
    if (size_ == 0) {
      push_construct(std::forward<Args>(args)...);
      return;
    }
    if (begin_.c_node_ + origin_ == 0) {
      allocation();
    }
    if (begin_.ptr_ == storage_[origin_ + begin_.c_node_]) {
      storage_[origin_ + begin_.c_node_ - 1] =
          alloc::allocate(alloc_, kNodeSize);
    }
    --begin_;
    safety_construct_front(std::forward<Args>(args)...);
    ++size_;
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

  const allocator_type& get_allocator() const { return alloc_; }
  allocator_type& get_allocator() { return alloc_; }
};

template <typename T, typename Allocator>
template <bool IsConst>
class Deque<T, Allocator>::base_iterator {
 public:
  using tp = std::conditional_t<IsConst, const T, T>;

  using difference_type = int64_t;
  using value_type = T;
  using pointer = tp*;
  using reference = tp&;
  using iterator_category = std::random_access_iterator_tag;

  operator base_iterator<false>() = delete;

 private:
  std::vector<T*>* storage_;
  T* ptr_;
  int64_t* origin_;
  int64_t c_node_;

  void deffault() {
    storage_ = nullptr;
    origin_ = nullptr;
    c_node_ = 0;
    ptr_ = nullptr;
  }

 public:
  base_iterator()
      : storage_(nullptr), ptr_(nullptr), origin_(nullptr), c_node_(0) {}

  base_iterator(std::vector<T*>* storage, T* ptr, int64_t* origin,
                int64_t c_node)
      : storage_(storage), ptr_(ptr), origin_(origin), c_node_(c_node) {}

  base_iterator(const base_iterator& other)
      : storage_(other.storage_),
        ptr_(other.ptr_),
        origin_(other.origin_),
        c_node_(other.c_node_) {}

  base_iterator(base_iterator&& other) {
    *this = other;
    other.deffault();
  }

  base_iterator& operator=(const base_iterator& other) {
    storage_ = other.storage_;
    origin_ = other.origin_;
    c_node_ = other.c_node_;
    ptr_ = other.ptr_;
    return *this;
  }

  base_iterator& operator=(base_iterator&& other) {
    *this = other;
    other.deffault();
    return *this;
  }

  base_iterator& operator++() {
    if (ptr_ - (*storage_)[*origin_ + c_node_] == kNodeSize - 1) {
      ptr_ = (*storage_)[*origin_ + (++c_node_)];
    } else {
      ++ptr_;
    }
    return *this;
  }

  base_iterator operator++(int) {
    base_iterator copy = *this;
    ++(*this);
    return copy;
  }

  base_iterator& operator--() {
    if (ptr_ - (*storage_)[*origin_ + c_node_] == 0) {
      ptr_ = (*storage_)[*origin_ + (--c_node_)] + kNodeSize - 1;
    } else {
      --ptr_;
    }
    return *this;
  }

  base_iterator operator--(int) const {
    base_iterator copy = *this;
    --(*this);
    return copy;
  }

  base_iterator& operator+=(int64_t shift) {
    if (storage_ != nullptr) {
      int64_t sgn = (shift >= 0 ? 1 : -1);
      int64_t sub_shift = ptr_ - (*storage_)[*origin_ + c_node_] +
                          sgn * ((sgn * shift) % kNodeSize);
      c_node_ +=
          shift / kNodeSize + (sub_shift >= 0 ? sub_shift / kNodeSize : -1);
      ptr_ = sub_shift >= 0
                 ? (*storage_)[*origin_ + c_node_] + sub_shift % kNodeSize
                 : (*storage_)[*origin_ + c_node_] + kNodeSize + sub_shift;
    }
    return *this;
  }

  base_iterator& operator-=(int64_t shift) { return *this += (-shift); }

  base_iterator operator-(int64_t shift) const {
    base_iterator copy = *this;
    return copy -= shift;
  }

  base_iterator operator+(int64_t shift) const {
    base_iterator copy = *this;
    return copy += shift;
  }

  bool operator<(const base_iterator& iter) const {
    if (c_node_ < iter.c_node_) {
      return true;
    }
    if (c_node_ == iter.c_node_) {
      return iter.ptr_ - ptr_ > 0;
    }
    return false;
  }

  bool operator>(const base_iterator& iter) const { return iter < *this; }

  bool operator<=(const base_iterator& iter) const { return !(*this > iter); }

  bool operator>=(const base_iterator& iter) const { return !(*this < iter); }

  bool operator==(const base_iterator& iter) const {
    return *this <= iter && *this >= iter;
  }

  bool operator!=(const base_iterator& iter) const { return !(iter == *this); }

  int64_t operator-(const base_iterator& iter) const {
    if (c_node_ == iter.c_node_) {
      return ptr_ - iter.ptr_;
    }
    int64_t sgn = ((c_node_ - iter.c_node_) > 0 ? 1 : -1);
    int64_t dist_1 = ptr_ - (*storage_)[*origin_ + c_node_] + 1;
    int64_t dist_2 = iter.ptr_ - (*storage_)[*origin_ + iter.c_node_] + 1;
    return (c_node_ - iter.c_node_ - 1 * sgn) * kNodeSize +
           sgn * (sgn * (dist_1 - dist_2) + kNodeSize);
  }

  reference operator*() const { return *ptr_; }

  pointer operator->() const { return ptr_; }

  void iter_swap(iterator& iter) {
    std::swap(ptr_, iter.ptr_);
    std::swap(c_node_, iter.c_node_);
  }

  friend class Deque;
};
