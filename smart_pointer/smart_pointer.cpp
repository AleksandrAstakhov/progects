#pragma once
#include <exception>
#include <memory>
#include <type_traits>

template <typename T>
class WeakPtr;

struct sp_count_base;
template <typename Y, typename Deleter, typename Allocator>
struct sp_ptr_count;
template <typename T>
struct sp_obj_count;
template <typename T, typename Allocator>
struct sp_allocated_obj;

struct sp_count_base {
  size_t shared_count;
  size_t weak_count;

  sp_count_base() : shared_count(1), weak_count(0) {}

  void add_shared_ref() noexcept { ++shared_count; }
  void add_weak_ref() noexcept { ++weak_count; }
  void remove_shared_ref() noexcept { --shared_count; }
  void remove_weak_ref() noexcept { --weak_count; }

  size_t use_count() const noexcept { return shared_count; }

  virtual void destroy() noexcept {}
  virtual void release() noexcept {}
  virtual void* get() noexcept { return nullptr; }
  virtual const void* get() const noexcept { return nullptr; }
};

template <typename T>
class SharedPtr {
  template <typename Y, typename... Args>
  friend SharedPtr<Y> MakeShared(Args&&... args);

  template <typename Y, typename Allocator, typename... Args>
  friend SharedPtr<Y> AllocateShared(const Allocator& alloc, Args&&... args);

  friend class WeakPtr<T>;

  template <typename Y>
  friend class SharedPtr;

  sp_count_base* m_base_ = nullptr;
  T* ptr_ = nullptr;

  SharedPtr(sp_count_base* count)
      : m_base_(count), ptr_(reinterpret_cast<T*>(count->get())) {}

  void destroy() noexcept {
    if (m_base_ == nullptr) {
      return;
    }
    m_base_->remove_shared_ref();
    if (m_base_->shared_count == 0) {
      m_base_->release();
    }
    if (m_base_->shared_count == 0 && m_base_->weak_count == 0) {
      m_base_->destroy();
    }
    m_base_ = nullptr;
  }

  void add_shared_ref() {
    if (m_base_ != nullptr) {
      m_base_->add_shared_ref();
    }
  }

 public:
  SharedPtr() = default;
  SharedPtr(std::nullptr_t) : m_base_(nullptr), ptr_(nullptr) {}

  template <typename Y>
  SharedPtr(const SharedPtr<Y>& shared) {
    if constexpr (std::is_base_of_v<T, Y> || std::is_same_v<T, Y>) {
      ptr_ = shared.ptr_;
      m_base_ = shared.m_base_;
      add_shared_ref();
    } else {
      throw std::runtime_error("error");
    }
  }

  SharedPtr(const SharedPtr& shared)
      : ptr_(shared.ptr_), m_base_(shared.m_base_) {
    add_shared_ref();
  }

  template <typename Y>
  SharedPtr(SharedPtr<Y>&& shared) {
    if constexpr (std::is_base_of_v<T, Y> || std::is_same_v<T, Y>) {
      ptr_ = shared.ptr_;
      m_base_ = shared.m_base_;
      shared.m_base_ = nullptr;
      shared.ptr_ = nullptr;
    } else {
      throw std::runtime_error("error");
    }
  }

  SharedPtr(SharedPtr&& shared) : ptr_(shared.ptr_), m_base_(shared.m_base_) {
    shared.m_base_ = nullptr;
    shared.ptr_ = nullptr;
  }

  template <typename Y, typename Deleter = std::default_delete<Y>,
            typename Allocator = std::allocator<Y>>
  SharedPtr(Y* ptr, Deleter del = Deleter(),
            const Allocator& alloc = Allocator()) {
    ptr_ = ptr;
    using sp_ptr_alloc = typename std::allocator_traits<
        Allocator>::template rebind_alloc<sp_ptr_count<Y, Deleter, Allocator>>;

    sp_ptr_alloc sp_alloc(alloc);
    m_base_ = std::allocator_traits<sp_ptr_alloc>::allocate(sp_alloc, 1);
    try {
      std::allocator_traits<sp_ptr_alloc>::construct(
          sp_alloc,
          reinterpret_cast<sp_ptr_count<Y, Deleter, Allocator>*>(m_base_), ptr,
          del, alloc);
    } catch (...) {
      std::allocator_traits<sp_ptr_alloc>::deallocate(
          sp_alloc,
          reinterpret_cast<sp_ptr_count<Y, Deleter, Allocator>*>(m_base_), 1);
      throw;
    }
  }

  template <typename Y>
  SharedPtr& operator=(const SharedPtr<Y>& shared) {
    if constexpr (std::is_base_of_v<T, Y> || std::is_same_v<T, Y>) {
      if (m_base_ == shared.m_base_) {
        return *this;
      }
      destroy();
      m_base_ = shared.m_base_;
      ptr_ = shared.ptr_;
      add_shared_ref();
    } else {
      throw std::runtime_error("error");
    }
    return *this;
  }

  SharedPtr& operator=(const SharedPtr& shared) {
    if (m_base_ == shared.m_base_) {
      return *this;
    }
    destroy();
    m_base_ = shared.m_base_;
    ptr_ = shared.ptr_;
    add_shared_ref();
    return *this;
  }

  template <typename Y>
  SharedPtr& operator=(SharedPtr<Y>&& shared) {
    if constexpr (std::is_base_of_v<T, Y> || std::is_same_v<T, Y>) {
      if (m_base_ == shared.m_base_) {
        return *this;
      }
      destroy();
      m_base_ = shared.m_base_;
      ptr_ = shared.ptr_;
      shared.m_base_ = nullptr;
      shared.ptr_ = nullptr;
    } else {
      throw std::runtime_error("error");
    }
    return *this;
  }

  SharedPtr& operator=(SharedPtr&& shared) {
    if (m_base_ == shared.m_base_) {
      return *this;
    }
    destroy();
    m_base_ = shared.m_base_;
    ptr_ = shared.ptr_;
    shared.m_base_ = nullptr;
    shared.ptr_ = nullptr;
    return *this;
  }

  T* get() noexcept { return ptr_; }

  const T* get() const noexcept { return ptr_; }

  size_t use_count() const noexcept {
    if (m_base_ == nullptr) {
      return 0;
    }
    return m_base_->use_count();
  }

  const T& operator*() const noexcept { return *(ptr_); }

  T& operator*() noexcept { return *(ptr_); }

  const T* operator->() const noexcept { return ptr_; }

  T* operator->() noexcept { return ptr_; }

  ~SharedPtr() { destroy(); }

  void reset() {
    destroy();
    m_base_ = nullptr;
    ptr_ = nullptr;
  }
};

template <typename Y, typename Deleter, typename Allocator>
struct sp_ptr_count final : sp_count_base {
  using alloc = std::allocator_traits<Allocator>;

  Y* m_ptr = nullptr;
  Deleter m_del;
  Allocator m_alloc;

  sp_ptr_count(Y* ptr, Deleter del, const Allocator& alloc)
      : sp_count_base(), m_ptr(ptr), m_del(std::move(del)), m_alloc(alloc) {}
  sp_ptr_count(const sp_ptr_count& other)
      : sp_count_base(),
        m_ptr(other.ptr_),
        m_del(other.m_del),
        m_alloc(other.m_alloc) {}
  sp_ptr_count(sp_ptr_count&& other)
      : sp_count_base(),
        m_ptr(other.ptr_),
        m_del(other.m_del),
        m_alloc(other.m_alloc) {
    other.ptr_ = nullptr;
  }

  virtual void release() noexcept override { m_del(m_ptr); }
  virtual void destroy() noexcept override {
    using sp_ptr_alloc = typename std::allocator_traits<
        Allocator>::template rebind_alloc<sp_ptr_count>;

    sp_ptr_alloc alloc(m_alloc);

    std::allocator_traits<sp_ptr_alloc>::deallocate(alloc, this, 1);
  }
  virtual void* get() noexcept override { return m_ptr; }
  virtual const void* get() const noexcept override { return m_ptr; }
};

template <typename T>
struct sp_obj_count final : sp_count_base {
  T m_obj;

  sp_obj_count(const T& obj) : sp_count_base(), m_obj(obj) {}
  sp_obj_count(T&& obj) : sp_count_base(), m_obj(std::move(obj)) {}
  template <typename... Args>
  sp_obj_count(Args&&... args)
      : sp_count_base(), m_obj(std::forward<Args>(args)...) {}

  virtual void release() noexcept override { m_obj.~T(); }
  virtual void destroy() noexcept override { operator delete(this); }
  virtual const void* get() const noexcept override { return &m_obj; }
  virtual void* get() noexcept override { return &m_obj; }
};

template <typename T, typename Allocator>
struct sp_allocated_obj final : sp_count_base {
  Allocator m_alloc;
  T m_obj;

  using my_alloc = typename std::allocator_traits<
      Allocator>::template rebind_alloc<sp_allocated_obj<T, Allocator>>;

  my_alloc alloc = m_alloc;

  sp_allocated_obj(const Allocator& alloc, const T& obj)
      : sp_count_base(), m_alloc(alloc), m_obj(obj) {}
  sp_allocated_obj(const Allocator& alloc, T&& obj)
      : sp_count_base(), m_alloc(alloc), m_obj(std::move(obj)) {}
  template <typename... Args>
  sp_allocated_obj(const Allocator& alloc, Args&&... args)
      : sp_count_base(), m_alloc(alloc), m_obj(std::forward<Args>(args)...) {}

  virtual void release() noexcept override {
    std::allocator_traits<my_alloc>::destroy(alloc, &m_obj);
  }
  virtual void destroy() noexcept override {
    std::allocator_traits<my_alloc>::deallocate(alloc, this, 1);
  }
  virtual const void* get() const noexcept override { return &m_obj; }
  virtual void* get() noexcept override { return &m_obj; }
};

template <typename Y, typename... Args>
SharedPtr<Y> MakeShared(Args&&... args) {
  return SharedPtr<Y>(reinterpret_cast<sp_count_base*>(
      new sp_obj_count<Y>(std::forward<Args>(args)...)));
}

template <typename Y, typename Allocator, typename... Args>
SharedPtr<Y> AllocateShared(const Allocator& alloc, Args&&... args) {
  using sp_allocator = typename std::allocator_traits<
      Allocator>::template rebind_alloc<sp_allocated_obj<Y, Allocator>>;

  sp_allocator sp_alloc(alloc);

  auto m_count = std::allocator_traits<sp_allocator>::allocate(sp_alloc, 1);
  try {
    std::allocator_traits<sp_allocator>::construct(sp_alloc, m_count, alloc,
                                                   std::forward<Args>(args)...);
  } catch (...) {
    std::allocator_traits<sp_allocator>::deallocate(sp_alloc, m_count, 1);
  }
  return SharedPtr<Y>(reinterpret_cast<sp_count_base*>(m_count));
}

template <typename T>
class WeakPtr {
  template <typename Y>
  friend class WeakPtr;

  sp_count_base* m_count_ = nullptr;

  void add_weak_ref() noexcept {
    if (m_count_ != nullptr) {
      m_count_->add_weak_ref();
    }
  }

  void destroy() {
    if (m_count_ == nullptr) {
      return;
    }
    m_count_->remove_weak_ref();
    if (m_count_->weak_count == 0 && m_count_->shared_count == 0) {
      m_count_->destroy();
    }
  }

 public:
  WeakPtr() = default;
  template <typename Y>
  WeakPtr(const WeakPtr<Y>& weak) {
    if constexpr (std::is_base_of_v<T, Y> || std::is_same_v<T, Y>) {
      m_count_ = weak.m_count_;
      add_weak_ref();
    } else {
      throw std::runtime_error("error");
    }
  }

  WeakPtr(const WeakPtr& weak) noexcept : m_count_(weak.m_count_) {
    add_weak_ref();
  }

  template <typename Y>
  WeakPtr(const SharedPtr<Y>& shared) {
    if constexpr (std::is_base_of_v<T, Y> || std::is_same_v<T, Y>) {
      m_count_ = shared.m_base_;
      add_weak_ref();
    } else {
      throw std::runtime_error("error");
    }
  }

  template <typename Y>
  WeakPtr(WeakPtr<Y>&& weak) noexcept : m_count_(weak.m_count_) {
    if constexpr (std::is_base_of_v<T, Y> || std::is_same_v<T, Y>) {
      m_count_ = weak.m_count_;
      weak.m_count_ = nullptr;
    } else {
      throw std::runtime_error("error");
    }
  }

  WeakPtr(WeakPtr&& weak) noexcept : m_count_(weak.m_count_) {
    weak.m_count_ = nullptr;
  }

  template <typename Y>
  WeakPtr& operator=(const WeakPtr<Y>& weak) noexcept {
    if constexpr (std::is_base_of_v<T, Y> || std::is_same_v<T, Y>) {
      destroy();
      m_count_ = weak.m_count_;
      add_weak_ref();
    } else {
      throw std::runtime_error("error");
    }
    return *this;
  }

  WeakPtr& operator=(const WeakPtr& weak) noexcept {
    destroy();
    m_count_ = weak.m_count_;
    add_weak_ref();
    return *this;
  }

  template <typename Y>
  WeakPtr& operator=(WeakPtr<Y>&& weak) noexcept {
    if constexpr (std::is_base_of_v<T, Y> || std::is_same_v<T, Y>) {
      destroy();
      m_count_ = weak.m_count_;
      weak.m_count_ = nullptr;
    } else {
      throw std::runtime_error("error");
    }
    return *this;
  }

  WeakPtr& operator=(WeakPtr&& weak) noexcept {
    destroy();
    m_count_ = weak.m_count_;
    weak.m_count_ = nullptr;
    return *this;
  }

  ~WeakPtr() { destroy(); }

  bool expired() const noexcept { return m_count_->shared_count == 0; }

  SharedPtr<T> lock() { return SharedPtr<T>(m_count_); }
};
