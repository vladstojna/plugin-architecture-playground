#include "blocking_queue.hpp"

namespace q {

template <typename T, size_t max_size>
template <size_t N>
blocking_queue<T, max_size>::releaser<N>::releaser(
    std::counting_semaphore<N> &sem)
    : sem(sem) {}

template <typename T, size_t max_size>
template <size_t N>
blocking_queue<T, max_size>::releaser<N>::~releaser() {
  sem.release();
}

template <typename T, size_t max_size>
blocking_queue<T, max_size>::blocking_queue()
    : queue_(), empty_count_(max_size), queued_count_(0) {}

template <typename T, size_t max_size>
void blocking_queue<T, max_size>::pop(T &x) {
  x = pop();
}

template <typename T, size_t max_size> T blocking_queue<T, max_size>::pop() {
  releaser rel(empty_count_);
  queued_count_.acquire();
  {
    std::scoped_lock lk(mtx_);
    T x = queue_.front();
    queue_.pop();
    return x;
  }
}

template <typename T, size_t max_size>
bool blocking_queue<T, max_size>::try_pop(T &x) {
  auto opt = try_pop();
  auto retval = bool{opt};
  if (retval)
    x = std::move(*opt);
  return retval;
}

template <typename T, size_t max_size>
std::optional<T> blocking_queue<T, max_size>::try_pop() {
  if (queued_count_.try_acquire()) {
    releaser rel(empty_count_);
    {
      std::scoped_lock lk(mtx_);
      T x = queue_.front();
      queue_.pop();
      return x;
    }
  }
  return std::nullopt;
}

template <typename T, size_t max_size>
blocking_queue<T, max_size>::size_type
blocking_queue<T, max_size>::size() const {
  std::scoped_lock lk(mtx_);
  return queue_.size();
}

template <typename T, size_t max_size>
bool blocking_queue<T, max_size>::empty() const {
  std::scoped_lock lk(mtx_);
  return queue_.empty();
}

template <typename T, size_t max_size>
template <typename U, typename>
void blocking_queue<T, max_size>::push(U &&x) {
  releaser rel(queued_count_);
  empty_count_.acquire();
  {
    std::scoped_lock lk(mtx_);
    queue_.push(std::forward<U>(x));
  }
}

template <typename T, size_t max_size>
template <typename U, typename>
bool blocking_queue<T, max_size>::try_push(U &&x) {
  if (empty_count_.try_acquire()) {
    releaser rel(queued_count_);
    {
      std::scoped_lock lk(mtx_);
      queue_.push(std::forward<U>(x));
      return true;
    }
  }
  return false;
}

template <typename T, size_t max_size>
template <typename... Args>
void blocking_queue<T, max_size>::emplace(Args &&...args) {
  releaser rel(queued_count_);
  empty_count_.acquire();
  {
    std::scoped_lock lk(mtx_);
    queue_.emplace(std::forward<Args>(args)...);
  }
}

template <typename T, size_t max_size>
template <typename... Args>
bool blocking_queue<T, max_size>::try_emplace(Args &&...args) {
  if (empty_count_.try_acquire()) {
    releaser rel(queued_count_);
    {
      std::scoped_lock lk(mtx_);
      queue_.emplace(std::forward<Args>(args)...);
      return true;
    }
  }
  return false;
}

} // namespace q
