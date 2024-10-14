#pragma once
#include <initializer_list>
#include <iterator>
#include <memory>
#include <type_traits>

template <typename T, typename Allocator = std::allocator<T>>
class List {
  template <bool IsConst>
  class base_iterator;
  struct node;
  struct base_node;

 public:
  using iterator = base_iterator<false>;
  using const_iterator = base_iterator<true>;

  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  using allocator_type = Allocator;

  using inner_alloc_type =
      typename std::allocator_traits<Allocator>::template rebind_alloc<node>;

  using alloc = std::allocator_traits<inner_alloc_type>;

  using value_type = T;

 private:
  size_t size_ = 0;
  base_node fake_node_;
  [[no_unique_address]] inner_alloc_type alloc_;

  template <typename... Args>
  void create_node(const iterator& iter, Args&&... args) {
    base_node* prev = iter.m_node_->m_prev;
    base_node* next = iter.m_node_;
    node* new_node = alloc::allocate(alloc_, 1);
    try {
      alloc::construct(alloc_, new_node, prev, next,
                       std::forward<Args>(args)...);
      next->m_prev = new_node;
      prev->m_next = new_node;
    } catch (...) {
      alloc::deallocate(alloc_, new_node, 1);
      throw;
    }
  }
  template <typename... Args>
  void safety_create_node(const iterator& iter, Args&&... args) {
    try {
      create_node(iter, std::forward<Args>(args)...);
    } catch (...) {
      destroy();
      throw;
    }
  }

  void delete_node(const iterator& iter) noexcept {
    base_node* prev = iter.m_node_->m_prev;
    base_node* next = iter.m_node_->m_next;
    alloc::destroy(alloc_, static_cast<node*>(iter.m_node_));
    alloc::deallocate(alloc_, static_cast<node*>(iter.m_node_), 1);
    prev->m_next = next;
    next->m_prev = prev;
  }

  void destroy() noexcept {
    while (fake_node_.m_next != &fake_node_) {
      delete_node(begin());
    }
    fake_node_.m_prev = &fake_node_;
    size_ = 0;
  }

 public:
  List() = default;
  List(size_t size, const T& value, const Allocator& alloc = Allocator())
      : size_(size), alloc_(alloc) {
    for (size_t ind = 0; ind < size; ind++) {
      safety_create_node(end(), value);
    }
  }

  explicit List(size_t size, const Allocator& alloc = Allocator())
      : size_(size), alloc_(alloc) {
    for (size_t ind = 0; ind < size; ind++) {
      safety_create_node(end());
    }
  }

  List(const List& list)
      : size_(list.size()),
        alloc_(alloc::select_on_container_copy_construction(list.alloc_)) {
    auto iter = list.begin();
    for (; iter != list.end(); iter++) {
      safety_create_node(end(), *iter);
    }
  }

  List(std::initializer_list<T> init, const Allocator& alloc = Allocator())
      : size_(init.size()), alloc_(alloc) {
    auto iter = init.begin();
    for (; iter != init.end(); iter++) {
      safety_create_node(end(), *iter);
    }
  }

  ~List() { destroy(); }

  List& operator=(const List& list) {
    List copy(list);
    destroy();
    if constexpr (alloc::propagate_on_container_copy_assignment::value) {
      alloc_ = list.alloc_;
    }
    copy.fake_node_.m_prev->m_next = copy.fake_node_.m_next->m_prev =
        &fake_node_;
    std::swap(fake_node_, copy.fake_node_);
    copy.fake_node_.m_prev = copy.fake_node_.m_next = &copy.fake_node_;
    size_ = list.size_;
    return *this;
  }

  T& front() { return *begin(); }
  const T& front() const { return *begin(); }

  T& back() { return *(--end()); }
  const T& back() const { return *(--end()); }

  void push_back(const T& value) {
    create_node(end(), value);
    ++size_;
  }
  void push_front(const T& value) {
    create_node(begin(), value);
    ++size_;
  }

  void push_back(T&& value) {
    create_node(end(), std::move(value));
    ++size_;
  }
  void push_front(T&& value) {
    create_node(begin(), std::move(value));
    ++size_;
  }

  void pop_back() noexcept {
    delete_node(--end());
    --size_;
  }
  void pop_front() noexcept {
    delete_node(begin());
    --size_;
  }

  size_t size() const noexcept { return size_; }
  bool empty() const noexcept { return size_ == 0; }

  inner_alloc_type& get_allocator() noexcept { return alloc_; }
  const inner_alloc_type& get_allocator() const noexcept { return alloc_; }

  iterator begin() noexcept { return iterator(fake_node_.m_next); }
  const_iterator begin() const noexcept {
    return const_iterator(fake_node_.m_next);
  }

  iterator end() noexcept { return iterator(&fake_node_); }
  const_iterator end() const noexcept { return const_iterator(&fake_node_); }

  const_iterator cbegin() const noexcept { return begin(); }
  const_iterator cend() const noexcept { return end(); }

  reverse_iterator rbegin() noexcept {
    return std::make_reverse_iterator(end());
  }
  const_reverse_iterator rbegin() const noexcept {
    return std::make_reverse_iterator(end());
  }

  reverse_iterator rend() noexcept {
    return std::make_reverse_iterator(begin());
  }
  const_reverse_iterator rend() const noexcept {
    return std::make_reverse_iterator(begin());
  }

  const_reverse_iterator crbegin() const noexcept { return rbegin(); }
  const_reverse_iterator crend() const noexcept { return rend(); }
};

template <typename T, typename Allocator>
struct List<T, Allocator>::base_node {
  base_node* m_prev = this;
  base_node* m_next = this;

  base_node() = default;
  base_node(base_node* prev, base_node* next) : m_prev(prev), m_next(next) {}
  base_node(const base_node& b_node)
      : m_prev(b_node.m_prev), m_next(b_node.m_next) {}
  base_node(base_node&& b_node) : m_prev(b_node.m_prev), m_next(b_node.m_next) {
    b_node.m_prev = b_node.m_next = &b_node;
  }
  base_node& operator=(const base_node& b_node) noexcept {
    m_prev = b_node.m_prev;
    m_next = b_node.m_next;
    return *this;
  }
  base_node& operator=(base_node&& b_node) noexcept {
    m_prev = b_node.m_prev;
    m_next = b_node.m_next;
    b_node.m_prev = b_node.m_next = &b_node;
    return *this;
  }
};

template <typename T, typename Allocator>
struct List<T, Allocator>::node final : base_node {
  T m_value;
  node() = default;
  template <typename... Args>
  node(base_node* prev, base_node* next, Args&&... args)
      : base_node(prev, next), m_value(std::forward<Args>(args)...) {}
};

template <typename T, typename Allocator>
template <bool IsConst>
class List<T, Allocator>::base_iterator {
  using tp = std::conditional_t<IsConst, const T, T>;
  using p_b_node = std::conditional_t<IsConst, base_node const*, base_node*>;
  using p_node = std::conditional_t<IsConst, node const*, node*>;

  friend class List<T, Allocator>;

  p_b_node m_node_;

 public:
  using value_type = T;
  using difference_type = int64_t;
  using reference = tp&;
  using pointer = tp*;
  using iterator_category = std::bidirectional_iterator_tag;

  base_iterator(p_b_node node) : m_node_(node) {}
  base_iterator(const base_iterator& iter) : m_node_(iter.m_node_) {}
  base_iterator(base_iterator&& iter) : m_node_(iter.m_node_) {
    iter.m_node_ = nullptr;
  }

  base_iterator& operator=(const base_iterator& iter) noexcept {
    m_node_ = iter.m_node_;
    return *this;
  }
  base_iterator& operator=(base_iterator&& iter) noexcept {
    m_node_ = iter.m_node_;
    iter.m_node_ = nullptr;
    return *this;
  }

  base_iterator& operator++() noexcept {
    m_node_ = m_node_->m_next;
    return *this;
  }
  base_iterator& operator--() noexcept {
    m_node_ = m_node_->m_prev;
    return *this;
  }

  base_iterator operator++(int) noexcept {
    base_iterator copy = *this;
    ++*this;
    return copy;
  }
  base_iterator operator--(int) noexcept {
    base_iterator copy = *this;
    --*this;
    return copy;
  }

  reference operator*() noexcept {
    return static_cast<p_node>(m_node_)->m_value;
  }
  reference operator*() const noexcept {
    return static_cast<p_node>(m_node_)->m_value;
  }

  pointer operator->() noexcept {
    return &(static_cast<p_node>(m_node_)->m_value);
  }
  pointer operator->() const noexcept {
    return &(static_cast<p_node>(m_node_)->m_value);
  }

  bool operator==(const base_iterator& iter) const noexcept {
    return m_node_ == iter.m_node_;
  }
  bool operator!=(const base_iterator& iter) const noexcept {
    return m_node_ != iter.m_node_;
  }
};
