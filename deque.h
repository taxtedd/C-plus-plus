#include <iostream>
#include <vector>

template <typename T>
class Deque {
 public:
  Deque();
  Deque(const Deque<T>& deque);
  Deque(int size);
  Deque(int size, const T& value);
  Deque<T>& operator=(const Deque<T>& deque);
  ~Deque();

  [[nodiscard]] size_t size() const;

  T& operator[](size_t index);
  const T& operator[](size_t index) const;
  T& at(ssize_t index);
  const T& at(ssize_t index) const;

  void push_back(const T& value);
  void pop_back();
  void push_front(const T& value);
  void pop_front();

  template <bool IsConst>
  class common_iterator;

  using iterator = common_iterator<false>;
  using const_iterator = common_iterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  iterator begin() { return iterator(&deque_[start_ / kBase], start_ % kBase); }
  const_iterator begin() const {
    return const_iterator(&deque_[start_ / kBase], start_ % kBase);
  }
  iterator end() {
    return iterator(&deque_[last_used_block(size_ + 1)],
                    last_in_block(last_used_block(size_ + 1), size_ + 1));
  }
  const_iterator end() const {
    return const_iterator(&deque_[last_used_block(size_ + 1)],
                          last_in_block(last_used_block(size_ + 1), size_ + 1));
  }

  const_iterator cbegin() const {
    return const_iterator(&deque_[start_ / kBase], start_ % kBase);
  }
  const_iterator cend() const {
    return const_iterator(&deque_[last_used_block(size_ + 1)],
                          last_in_block(last_used_block(size_ + 1), size_ + 1));
  }

  reverse_iterator rbegin() { return std::make_reverse_iterator(end()); }
  const_reverse_iterator rbegin() const {
    return std::make_reverse_iterator(cend());
  }
  const_reverse_iterator crbegin() const {
    return std::make_reverse_iterator(cend());
  }

  reverse_iterator rend() { return std::make_reverse_iterator(begin()); }
  const_reverse_iterator rend() const {
    return std::make_reverse_iterator(cbegin());
  }
  const_reverse_iterator crend() const {
    return std::make_reverse_iterator(cbegin());
  }

  void insert(iterator iter, const T& element);
  void erase(iterator iter);

 private:
  std::vector<T*> deque_;
  size_t size_{0};
  size_t allocated_blocks_{0};
  size_t start_{0};
  static const int kBase = 32;

  size_t first_in_block(size_t index) const {
    return (index == block(start_) ? start_ % kBase : 0);
  }
  size_t last_in_block(size_t index, size_t size) const {
    if (size == 0) {
      return 0;
    }
    return (index == last_used_block(size) ? (start_ + size - 1) % kBase
                                           : kBase - 1);
  }
  size_t block(size_t index) const { return index / kBase; }
  size_t last_used_block(size_t size) const {
    if (size == 0) {
      return 0;
    }
    return (start_ + size - 1) / kBase;
  }

  void memory_allocation() {
    size_t number_of_used_blocks = (size_ + kBase - 1) / kBase;
    if (size_ == 0) {
      number_of_used_blocks = 1;
    }
    std::vector<T*> temporary(number_of_used_blocks * 3);
    for (size_t i = 0; i < number_of_used_blocks; i++) {
      temporary[i] = (reinterpret_cast<T*>(new char[kBase * sizeof(T)]));
      temporary[i + number_of_used_blocks] = deque_[block(start_) + i];
    }
    for (size_t i = number_of_used_blocks * 2; i < (number_of_used_blocks * 3);
         i++) {
      temporary[i] = (reinterpret_cast<T*>(new char[kBase * sizeof(T)]));
    }

    for (size_t i = 0; i < block(start_); i++) {
      delete[] reinterpret_cast<char*>(deque_[i]);
    }
    for (size_t i = last_used_block(size_) + 1; i < allocated_blocks_; i++) {
      delete[] reinterpret_cast<char*>(deque_[i]);
    }

    deque_.swap(temporary);
    allocated_blocks_ = number_of_used_blocks * 3;
    start_ = number_of_used_blocks * kBase;
  }

  void zero_allocation() {
    deque_.push_back(reinterpret_cast<T*>(new char[kBase * sizeof(T)]));
    allocated_blocks_ = 1;
  }
};
template <typename T>
Deque<T>::Deque() {
  zero_allocation();
}

template <typename T>
Deque<T>::Deque(const Deque<T>& deque)
    : deque_(std::vector<T*>(deque.allocated_blocks_)),
      size_(deque.size_),
      allocated_blocks_(deque.allocated_blocks_),
      start_(deque.start_) {
  try {
    for (size_t i = 0; i < allocated_blocks_; i++) {
      deque_[i] = reinterpret_cast<T*>(new char[kBase * sizeof(T)]);
      for (size_t j = first_in_block(i); j <= last_in_block(i, size_); j++) {
        new (deque_[i] + j) T(deque.deque_[i][j]);
      }
    }
  } catch (...) {
    throw;
  }
}
template <typename T>
Deque<T>::Deque(int size)
    : deque_(std::vector<T*>((size + kBase - 1) / kBase)),
      size_(size),
      allocated_blocks_((size + kBase - 1) / kBase),
      start_(0) {
  for (size_t i = 0; i < allocated_blocks_; i++) {
    deque_[i] = reinterpret_cast<T*>(new char[kBase * sizeof(T)]);
    for (size_t j = 0; j <= last_in_block(i, size_); j++) {
      try {
        new (deque_[i] + j) T();
      } catch (...) {
        for (size_t k = 0; k < i; ++k) {
          for (size_t len = 0; len < kBase; len++) {
            (deque_[k] + len)->~T();
          }
        }
        for (size_t len = 0; len < j; len++) {
          (deque_[i] + len)->~T();
        }
        for (size_t k = 0; k <= i; ++k) {
          delete[] reinterpret_cast<char*>(deque_[k]);
        }
        throw;
      }
    }
  }
}
template <typename T>
Deque<T>::Deque(int size, const T& value)
    : deque_(std::vector<T*>((size + kBase - 1) / kBase)),
      size_(size),
      allocated_blocks_((size + kBase - 1) / kBase),
      start_(0) {
  for (size_t i = 0; i < allocated_blocks_; i++) {
    deque_[i] = reinterpret_cast<T*>(new char[kBase * sizeof(T)]);
    for (size_t j = 0; j <= last_in_block(i, size_); j++) {
      try {
        new (deque_[i] + j) T(value);
      } catch (...) {
        for (size_t k = 0; k < i; k++) {
          for (size_t len = 0; len < kBase; len++) {
            (deque_[k] + len)->~T();
          }
        }
        for (size_t len = 0; len < j; len++) {
          (deque_[i] + len)->~T();
        }
        for (size_t k = 0; k <= i; ++k) {
          delete[] reinterpret_cast<char*>(deque_[k]);
        }
        throw;
      }
    }
  }
}
template <typename T>
Deque<T>& Deque<T>::operator=(const Deque<T>& deque) {
  Deque temporary(deque);
  std::swap(temporary.deque_, deque_);
  std::swap(temporary.size_, size_);
  std::swap(temporary.allocated_blocks_, allocated_blocks_);
  std::swap(temporary.start_, start_);
  return *this;
}
template <typename T>
Deque<T>::~Deque() {
  for (size_t i = 0; i < size_; i++) {
    operator[](i).~T();
  }
  for (size_t i = 0; i < allocated_blocks_; i++) {
    delete[] reinterpret_cast<char*>(deque_[i]);
  }
}

template <typename T>
size_t Deque<T>::size() const {
  return size_;
}

template <typename T>
T& Deque<T>::operator[](size_t index) {
  return deque_[(start_ + index) / kBase][(start_ + index) % kBase];
}
template <typename T>
const T& Deque<T>::operator[](size_t index) const {
  return deque_[(start_ + index) / kBase][(start_ + index) % kBase];
}
template <typename T>
T& Deque<T>::at(ssize_t index) {
  if (index < 0 or index >= static_cast<ssize_t>(size_)) {
    throw std::out_of_range("");
  }
  return deque_[block(start_ + index)][(start_ + index) % kBase];
}
template <typename T>
const T& Deque<T>::at(ssize_t index) const {
  if (index < 0 or index >= size_) {
    throw std::out_of_range("");
  }
  return deque_[block(start_ + index)][(start_ + index) % kBase];
}

template <typename T>
void Deque<T>::push_back(const T& value) {
  try {
    if (allocated_blocks_ == 0) {
      zero_allocation();
    }
    if (last_used_block(size_) + 1 == allocated_blocks_ and
        last_in_block(last_used_block(size_), size_) == (kBase - 1)) {
      memory_allocation();
    }
  } catch (...) {
    throw;
  }
  new (deque_[last_used_block(size_ + 1)] +
       last_in_block(last_used_block(size_ + 1), size_ + 1)) T(value);
  size_++;
}
template <typename T>
void Deque<T>::pop_back() {
  (deque_[last_used_block(size_)] +
   last_in_block(last_used_block(size_), size_))
      ->~T();
  size_--;
}
template <typename T>
void Deque<T>::push_front(const T& value) {
  try {
    if (allocated_blocks_ == 0) {
      zero_allocation();
    }
    if (start_ == 0) {
      memory_allocation();
    }
  } catch (...) {
    throw;
  }
  size_++;
  start_--;
  new (deque_[block(start_)] + first_in_block(block(start_))) T(value);
}
template <typename T>
void Deque<T>::pop_front() {
  (deque_[block(start_)] + first_in_block(block(start_)))->~T();
  size_--;
  start_++;
}

template <typename T>
template <bool IsConst>
class Deque<T>::common_iterator {
 public:
  using value_type = std::conditional_t<IsConst, const T, T>;
  using difference_type = long long;
  using pointer = value_type*;
  using reference = value_type&;
  using iterator_category = std::random_access_iterator_tag;
  using block_pointer = std::conditional_t<IsConst, T* const*, T**>;

  common_iterator(block_pointer block, long long index)
      : block_(block), index_(index) {}
  operator const_iterator() { return const_iterator(*this); }

  reference operator*() { return (*block_)[index_]; }
  pointer operator->() { return &((*block_)[index_]); }

  common_iterator& operator+=(int number) {
    block_ += (index_ + number) / kBase;
    index_ = (index_ + number) % kBase;
    if (index_ < 0) {
      --block_;
      index_ += kBase;
    }
    return *this;
  }
  common_iterator& operator-=(int number) {
    *this += (-number);
    return *this;
  }

  common_iterator operator+(int number) const {
    auto it_copy = *this;
    it_copy += number;
    return it_copy;
  }
  common_iterator operator-(int number) const {
    auto it_copy = *this;
    it_copy -= number;
    return it_copy;
  }

  common_iterator& operator++() { return (*this += 1); }
  common_iterator operator++(int) {
    auto iter = *this;
    *this += 1;
    return (iter);
  }
  common_iterator& operator--() { return (*this -= 1); }
  common_iterator operator--(int) {
    auto iter = *this;
    *this -= 1;
    return (iter);
  }

  bool operator<(const common_iterator& iter) const {
    return (block_ < iter.block_ or
            (block_ == iter.block_ and index_ < iter.index_));
  }
  bool operator>(const common_iterator& iter) const { return iter < (*this); }
  bool operator<=(const common_iterator& iter) const {
    return !((*this) > iter);
  }
  bool operator>=(const common_iterator& iter) const {
    return !((*this) < iter);
  }
  bool operator==(const common_iterator& iter) const {
    return (block_ == iter.block_ and index_ == iter.index_);
  }
  bool operator!=(const common_iterator& iter) const {
    return !((*this) == iter);
  }

  difference_type operator-(const common_iterator& iter) const {
    return (block_ - iter.block_) * kBase + index_ - iter.index_;
  }

 private:
  block_pointer block_;
  long long index_ = 0;
};

template <typename T>
void Deque<T>::insert(iterator iter, const T& element) {
  size_t size1 = iter - begin();
  if (allocated_blocks_ == 0) {
    zero_allocation();
  }
  if (last_used_block(size_) + 1 == allocated_blocks_ and
      last_in_block(last_used_block(size_), size_) == (kBase - 1)) {
    memory_allocation();
  }
  iter = iterator(&deque_[(start_ + size1) / kBase], (start_ + size1) % kBase);
  for (auto cur_it = end(); cur_it > iter; cur_it--) {
    *cur_it = *(cur_it - 1);
  }
  *iter = T(element);
  size_++;
}

template <typename T>
void Deque<T>::erase(Deque<T>::iterator iter) {
  for (auto cur_it = iter; cur_it < end() - 1; cur_it++) {
    *cur_it = *(cur_it + 1);
  }
  pop_back();
}
