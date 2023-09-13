// NOLINTBEGIN
#include <iostream>
#include <memory>

template <typename T, typename Allocator = std::allocator<T>>
class List {
 private:
  struct BaseNode;

  struct Node : BaseNode {
    T value;

    Node() = default;

    Node(const T& value) : value(value) {}
  };

  BaseNode fakeNode_;
  Node* head_ = static_cast<Node*>(&fakeNode_);
  Node* tail_ = static_cast<Node*>(&fakeNode_);
  size_t size_ = 0;

  using NodeAlloc =
      typename std::allocator_traits<Allocator>::template rebind_alloc<Node>;
  using AllocTraits = std::allocator_traits<NodeAlloc>;

  NodeAlloc alloc_;

  void add_node_to_end(Node* new_node) {
    new_node->prev = tail_->prev;
    new_node->next = tail_;
    if (tail_->prev != nullptr) {
      tail_->prev->next = new_node;
    } else {
      head_ = new_node;
    }

    tail_->prev = new_node;
    size_++;
  }

  void add_node_to_start(Node* new_node) {
    new_node->next = head_;
    head_->prev = new_node;
    head_ = new_node;

    size_++;
  }

  void swap_lists(List<T, Allocator>& list) {
    std::swap(list.fakeNode_, fakeNode_);
    std::swap(list.size_, size_);
    std::swap(list.head_, head_);
    std::swap(list.tail_, tail_);
    std::swap(list.alloc_, alloc_);
  }

 public:
  List() = default;
  List(size_t size, const Allocator& alloc = Allocator());
  List(size_t size, const T& value, const Allocator& alloc = Allocator());
  List(const Allocator& alloc);
  List(const List<T, Allocator>& list);
  List& operator=(const List& list);
  ~List();

  void push_back(const T& value);
  void push_front(const T& value);
  void pop_back();
  void pop_front();

  [[nodiscard]] size_t size() const { return size_; }

  NodeAlloc get_allocator() const { return alloc_; }

  template <bool is_const>
  class common_iterator;

  using iterator = common_iterator<false>;
  using const_iterator = common_iterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  iterator begin() { return iterator(head_, 0); }

  iterator end() { return iterator(tail_, size_); }

  const_iterator begin() const { return const_iterator(head_, 0); }

  const_iterator end() const { return const_iterator(tail_, size_); }

  const_iterator cbegin() const { return begin(); }

  const_iterator cend() const { return end(); }

  reverse_iterator rbegin() { return std::make_reverse_iterator(end()); }

  reverse_iterator rend() { return std::make_reverse_iterator(begin()); }

  const_reverse_iterator rbegin() const {
    return std::make_reverse_iterator(cend());
  }

  const_reverse_iterator rend() const {
    return std::make_reverse_iterator(cbegin());
  }

  const_reverse_iterator crbegin() const {
    return std::make_reverse_iterator(cend());
  }

  const_reverse_iterator crend() const {
    return std::make_reverse_iterator(cbegin());
  }

  void insert(const_iterator it, const T& element);

  void add_node_to_pos(const_iterator it, Node* new_node) {
    if (it->prev == nullptr) {
      head_->prev = new_node;
      new_node->next = head_;
      head_ = new_node;
      if (size_ == 0) {
        head_->next = nullptr;
      }
    } else {
      it->prev->next = new_node;
      new_node->prev = it->prev;
      new_node->next = it.get_node();
    }
    it->prev = new_node;

    size_++;
  }

  void erase(const_iterator it);
};

template <typename T, typename Allocator>
struct List<T, Allocator>::BaseNode {
  Node* next = nullptr;
  Node* prev = nullptr;
};

template <typename T, typename Allocator>
List<T, Allocator>::List(size_t size, const Allocator& alloc)
    : size_(0), alloc_(alloc) {
  while (size_ != size) {
    Node* new_node(AllocTraits::allocate(alloc_, 1));
    try {
      AllocTraits::construct(alloc_, new_node);
    } catch (...) {
      AllocTraits::deallocate(alloc_, new_node, 1);
      while (size_ > 0) {
        pop_front();
      }
      throw;
    }
    add_node_to_end(new_node);
  }
}

template <typename T, typename Allocator>
List<T, Allocator>::List(size_t size, const T& value, const Allocator& alloc)
    : size_(0), alloc_(alloc) {
  try {
    while (size_ != size) {
      this->push_back(value);
    }
  } catch (...) {
    for (size_t j = 0; j < size_; j++) {
      pop_front();
    }
  }
}

template <typename T, typename Allocator>
List<T, Allocator>::List(const Allocator& alloc) : alloc_(alloc) {}

template <typename T, typename Allocator>
List<T, Allocator>::List(const List<T, Allocator>& list)
    : List(AllocTraits::select_on_container_copy_construction(list.alloc_)) {
  for (auto& value : list) {
    this->push_back(value);
  }
}

template <typename T, typename Alloc>
List<T, Alloc>& List<T, Alloc>::operator=(const List<T, Alloc>& list) {
  List temporary(list);

  if (AllocTraits::propagate_on_container_copy_assignment::value) {
    temporary.alloc_ = list.alloc_;
  }

  swap_lists(temporary);
  return *this;
}

template <typename T, typename Allocator>
List<T, Allocator>::~List() {
  while (size_ != 0) {
    pop_front();
  }
}

template <typename T, typename Allocator>
void List<T, Allocator>::push_back(const T& value) {
  size_t size = size_;
  Node* new_node(AllocTraits::allocate(alloc_, 1));
  try {
    AllocTraits::construct(alloc_, new_node, value);
    add_node_to_end(new_node);
  } catch (...) {
    AllocTraits::deallocate(alloc_, new_node, 1);
    size_ = size;
    throw;
  }
}

template <typename T, typename Allocator>
void List<T, Allocator>::push_front(const T& value) {
  size_t size = size_;
  Node* new_node(AllocTraits::allocate(alloc_, 1));
  try {
    AllocTraits::construct(alloc_, new_node, value);
    add_node_to_start(new_node);
  } catch (...) {
    AllocTraits::deallocate(alloc_, new_node, 1);
    size_ = size;
    throw;
  }
}

template <typename T, typename Allocator>
void List<T, Allocator>::pop_back() {
  if (size_ == 1) {
    tail_->prev = nullptr;
    AllocTraits::destroy(alloc_, head_);
    AllocTraits::deallocate(alloc_, head_, 1);
    head_ = tail_;
  } else {
    Node* tail = (tail_->prev)->prev;
    AllocTraits::destroy(alloc_, tail_->prev);
    AllocTraits::deallocate(alloc_, tail_->prev, 1);
    tail->next = tail_;
    tail_->prev = tail;
  }

  size_--;
}

template <typename T, typename Allocator>
void List<T, Allocator>::pop_front() {
  Node* head = head_->next;
  AllocTraits::destroy(alloc_, head_);
  AllocTraits::deallocate(alloc_, head_, 1);
  head_ = head;
  head_->prev = nullptr;

  size_--;
}

template <typename T, typename Allocator>
template <bool is_const>
class List<T, Allocator>::common_iterator {
 public:
  using value_type = std::conditional_t<is_const, const T, T>;
  using pointer = value_type*;
  using reference = value_type&;
  using iterator_category = std::bidirectional_iterator_tag;
  using difference_type = long long;

  common_iterator(Node* node, long long index) : node_(node), index_(index) {}

  operator const_iterator() const { return const_iterator(node_, index_); }

  reference operator*() { return node_->value; }

  Node* operator->() { return node_; }

  Node* get_node() { return node_; }

  common_iterator& operator++() {
    index_ += 1;
    node_ = node_->next;
    return *this;
  }

  common_iterator operator++(int) {
    auto it = *this;
    ++*this;
    return (it);
  }

  common_iterator& operator--() {
    index_ -= 1;
    node_ = node_->prev;
    return *this;
  }

  common_iterator operator--(int) {
    auto it = *this;
    --*this;
    return (it);
  }

  bool operator==(const common_iterator& it) const {
    return (index_ == it.index_);
  }

  bool operator!=(const common_iterator& it) const { return !((*this) == it); }

 private:
  Node* node_;
  long long index_ = 0;
};

template <typename T, typename Allocator>
void List<T, Allocator>::insert(const_iterator it, const T& element) {
  size_t size = size_;
  try {
    Node* new_node(AllocTraits::allocate(alloc_, 1));
    AllocTraits::construct(alloc_, new_node, element);
    add_node_to_pos(it, new_node);
  } catch (...) {
    size_ = size;
  }
}

template <typename T, typename Allocator>
void List<T, Allocator>::erase(const_iterator it) {
  if (it->prev == nullptr) {
    it->next->prev = nullptr;
    head_ = it->next;
  } else {
    it->prev->next = it->next;
    it->next->prev = it->prev;
  }

  AllocTraits::destroy(alloc_, it.get_node());
  AllocTraits::deallocate(alloc_, it.get_node(), 1);

  size_--;
}

template <size_t N>
struct StackStorage {
  StackStorage() = default;
  StackStorage(const StackStorage<N>& stack_storage) = delete;
  StackStorage& operator=(const StackStorage<N>& stack_storage) = delete;
  ~StackStorage() = default;

  char array_[N];
  size_t last_used_ = 0;
};

template <typename T, size_t N>
struct StackAllocator {
  using value_type = T;
  using pointer = value_type*;

  template <class U>
  struct rebind {
    using other = StackAllocator<U, N>;
  };

  template <class U>
  StackAllocator(const StackAllocator<U, N>& other)
      : storage_(other.storage_) {}

  StackAllocator() = default;

  StackAllocator(StackStorage<N>& stack_storage) : storage_(&stack_storage){};

  StackAllocator(const StackAllocator<T, N>& alloc)
      : storage_(alloc.storage_){};
  StackAllocator& operator=(const StackAllocator<T, N>& alloc);
  ~StackAllocator() = default;

  pointer allocate(const size_t kN);

  void deallocate(const T*, const size_t){};

  bool operator==(const StackAllocator& alloc) {
    return storage_ == alloc.storage_;
  }

  bool operator!=(const StackAllocator& alloc) { return !(alloc == *this); }

  StackStorage<N>* storage_;
};

template <typename T, size_t N>
typename StackAllocator<T, N>::pointer StackAllocator<T, N>::allocate(
    const size_t kN) {
  void* begin = storage_->array_ + storage_->last_used_;
  size_t free = N - storage_->last_used_;
  if (std::align(alignof(value_type), kN * sizeof(value_type), begin, free)) {
    storage_->last_used_ =
        reinterpret_cast<char*>(reinterpret_cast<char*>(begin) +
                                kN * sizeof(value_type)) -
        storage_->array_;
    return reinterpret_cast<value_type*>(begin);
  }
  throw std::bad_alloc();
}

template <typename T, size_t N>
StackAllocator<T, N>& StackAllocator<T, N>::operator=(
    const StackAllocator<T, N>& alloc) {
  StackAllocator temporary(alloc);
  std::swap(storage_, temporary.storage_);
  return *this;
}
// NOLINTEND
