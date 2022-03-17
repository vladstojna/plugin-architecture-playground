#pragma once

#include <mutex>
#include <optional>
#include <queue>
#include <semaphore>

namespace q {

template <typename T, size_t max_size = 256> class blocking_queue {
public:
  using value_type = T;
  using size_type = std::queue<value_type>::size_type;

  blocking_queue();
  blocking_queue(const blocking_queue &) = delete;
  blocking_queue &operator=(const blocking_queue &) = delete;

  value_type pop();
  void pop(value_type &);

  std::optional<value_type> try_pop();
  bool try_pop(value_type &);

  template <typename U, typename = std::enable_if_t<std::is_convertible_v<
                            std::remove_cvref_t<U>, value_type>>>
  void push(U &&);

  template <typename U, typename = std::enable_if_t<std::is_convertible_v<
                            std::remove_cvref_t<U>, value_type>>>
  bool try_push(U &&);

  template <typename... Args> void emplace(Args &&...);
  template <typename... Args> bool try_emplace(Args &&...);

  size_type size() const;
  bool empty() const;

private:
  template <size_t N> struct releaser {
    std::counting_semaphore<N> &sem;

    releaser(std::counting_semaphore<N> &);
    ~releaser();
  };

  mutable std::mutex mtx_;
  std::queue<value_type> queue_;
  std::counting_semaphore<max_size> empty_count_;
  std::counting_semaphore<max_size> queued_count_;
};

} // namespace q

#include "blocking_queue.inl"
