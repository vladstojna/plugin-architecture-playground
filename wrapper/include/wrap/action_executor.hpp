#pragma once

#include <wrap/action.hpp>

#include <functional>
#include <mutex>
#include <system_error>

namespace wrap {

class payload;
struct error_descriptor;

class action_executor {
public:
  using on_finish_t = std::function<void(std::error_code, std::string_view)>;
  using on_error_t = std::function<void(std::exception_ptr)>;

  action_executor(const action &, on_finish_t, on_error_t = {});

  action_executor(const action_executor &) = delete;
  action_executor &operator=(const action_executor &) = delete;

  ~action_executor();

  const action &get_action() const noexcept { return action_; }

  void submit(const payload &, error_descriptor &);
  bool submit(const payload &, std::error_code &, error_descriptor &) noexcept;

  void await();

private:
  struct helper;
  friend helper;

  action action_;
  on_finish_t on_finish_;
  on_error_t on_error_;

  std::condition_variable submitcv_;
  std::atomic<int32_t> submitctr_;
};

} // namespace wrap
