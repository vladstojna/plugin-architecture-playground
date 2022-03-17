#include "..\include\wrap\action_executor.hpp"
#include <wrap/action_executor.hpp>
#include <wrap/error.hpp>
#include <wrap/error_descriptor.hpp>
#include <wrap/payload.hpp>

#include <module_load/module.hpp>

#include "status_utils.hpp"

namespace wrap {

struct action_executor::helper {
  static void wrap_callback(PASMP_status_t s, PASMP_string_view_t err_msg,
                            void *data) {
    action_executor &exec = *static_cast<action_executor *>(data);
    try {
      exec.on_finish_(make_error_code(s), {err_msg.data, err_msg.size});
    } catch (...) {
      try {
        if (exec.on_error_)
          exec.on_error_(std::current_exception());
        else
          handle_callback_exception();
      } catch (...) {
        handle_callback_exception();
      }
    }
    exec.submitctr_.fetch_sub(1);
    exec.submitcv_.notify_one();
  }
};

action_executor::action_executor(const action &a, on_finish_t of, on_error_t oe)
    : action_(a), on_finish_(std::move(of)), on_error_(std::move(oe)),
      submitctr_(0) {}

action_executor::~action_executor() { await(); }

void action_executor::submit(const payload &p, error_descriptor &ed) {
  if (std::error_code ec; !submit(p, ec, ed))
    error_code_as_exception(ec, ed);
}

bool action_executor::submit(const payload &p, std::error_code &ec,
                             error_descriptor &ed) noexcept {
  const auto &mod = get_action().get_plugin().get_module();
  auto handle_plugin = get_action().get_plugin().get();
  auto handle_action = get_action().get();
  ed.clear();
  auto status = mod.funcs().action_execute_async(
      handle_plugin, handle_action, p, &helper::wrap_callback, this, &ed);
  ec = make_error_code(status);
  if (!ec)
    submitctr_.fetch_add(1);
  return !ec;
}

void action_executor::await() {
  std::mutex mux;
  std::unique_lock lk{mux};
  submitcv_.wait(lk, [this]() { return submitctr_.load() <= 0; });
}

} // namespace wrap
