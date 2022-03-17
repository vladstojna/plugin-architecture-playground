#include <module_load/exception.hpp>

namespace modl {

module_incompatible::module_incompatible(library_version current,
                                         library_version found) noexcept
    : current_(current), found_(found) {}

library_version module_incompatible::current() const noexcept {
  return current_;
}

library_version module_incompatible::found() const noexcept { return found_; }

const char *module_incompatible::what() const noexcept {
  return "Incompatible library versions";
}

struct function_load_error::context {
  std::error_code code;
  std::string name;

  context(std::error_code &&ec, std::string_view n)
      : name(n), code(std::move(ec)) {}
};

function_load_error::function_load_error(std::error_code ec,
                                         std::string_view name)
    : ctx_(std::make_shared<context>(std::move(ec), name)) {}

const char *function_load_error::name() const noexcept {
  return ctx_->name.c_str();
}

std::error_code function_load_error::code() const noexcept {
  return ctx_->code;
}

const char *function_load_error::what() const noexcept {
  return "Error loading module function";
}

} // namespace modl
