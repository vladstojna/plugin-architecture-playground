#pragma once

#include <wrap/action_event.hpp>
#include <wrap/passkey.hpp>
#include <wrap/visibility.hpp>

#include <filesystem>
#include <functional>

namespace wrap {

class plugin;
class action;

class WRAPPER_DLL_PUBLIC plugin_attributes {
public:
  using callback_t = std::function<void(action_event, action)>;
  using on_error_t = std::function<void(std::exception_ptr)>;

  plugin_attributes(std::filesystem::path p, callback_t c,
                    on_error_t e = {}) noexcept
      : path_(std::move(p)), callback_(std::move(c)), on_error_(std::move(e)){};

  const callback_t &callback(key<plugin>) const noexcept { return callback_; };
  const on_error_t &on_error(key<plugin>) const noexcept { return on_error_; };

  const std::filesystem::path &persistence_path() const noexcept {
    return path_;
  };

private:
  std::filesystem::path path_;
  callback_t callback_;
  on_error_t on_error_;
};

} // namespace wrap
