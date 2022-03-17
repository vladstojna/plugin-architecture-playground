#include "..\include\wrap\error.hpp"
#include <wrap/error.hpp>
#include <wrap/error_descriptor.hpp>

#include <cassert>
#include <iostream>
#include <string>

#pragma warning(error : 4062)

namespace {

auto default_error_handler_v =
    +[](const char *msg, std::error_code ec) noexcept {
      std::cerr << msg << " (" << ec << ")" << std::endl;
    };

struct logic_category_t : std::error_category {
  const char *name() const noexcept override;
  std::string message(int) const override;
};

struct generic_category_t : std::error_category {
  const char *name() const noexcept override;
  std::string message(int) const override;
};

struct plugin_category_t : std::error_category {
  const char *name() const noexcept override;
  std::string message(int) const override;
};

struct action_category_t : std::error_category {
  const char *name() const noexcept override;
  std::string message(int) const override;
};

const logic_category_t logic_category_v;
const generic_category_t generic_category_v;
const plugin_category_t plugin_category_v;
const action_category_t action_category_v;

const char *logic_category_t::name() const noexcept { return "logic"; }

const char *generic_category_t::name() const noexcept { return "generic"; }

const char *plugin_category_t::name() const noexcept { return "plugin"; }

const char *action_category_t::name() const noexcept { return "action"; }

std::string logic_category_t::message(int cv) const {
  switch (static_cast<wrap::logic_errc>(cv)) {
  case wrap::logic_errc::invalid_status:
    return "invalid status value";
  case wrap::logic_errc::invalid_argument:
    return "invalid argument in plugin call";
  case wrap::logic_errc::unknown:
    return "unknown error in plugin call";
  case wrap::logic_errc::fatal:
    return "fatal error in plugin call";
  case wrap::logic_errc::callback_exception:
    return "exception in user-provided callback";
  }
  return "(unrecognized error code)";
}

std::string generic_category_t::message(int cv) const {
  switch (static_cast<wrap::generic_errc>(cv)) {
  case wrap::generic_errc::not_implemented:
    return "feature not implemented";
  case wrap::generic_errc::alloc:
    return "error allocating memory in plugin";
  case wrap::generic_errc::system:
    return "system error";
  case wrap::generic_errc::truncated:
    return "input buffer truncated";
  case wrap::generic_errc::other:
    return "other error; check descriptor for more information";
  }
  return "(unrecognized error code)";
}

std::string plugin_category_t::message(int cv) const {
  switch (static_cast<wrap::plugin_errc>(cv)) {
  case wrap::plugin_errc::action_not_found:
    return "action not found";
  case wrap::plugin_errc::unavailable:
    return "request currently not available";
  case wrap::plugin_errc::other:
    return "other error; check descriptor for more information";
  case wrap::plugin_errc::bad_persistence_path:
    return "bad persistence path (must exist, be a directory and have "
           "write "
           "permissions)";
  }
  return "(unrecognized error code)";
}

std::string action_category_t::message(int cv) const {
  switch (static_cast<wrap::action_errc>(cv)) {
  case wrap::action_errc::execution:
    return "error during action execution";
  case wrap::action_errc::serialization:
    return "error during action (de)serialization";
  case wrap::action_errc::invalid_payload:
    return "invalid payload tag";
  case wrap::action_errc::other:
    return "other error; check descriptor for more information";
  }
  return "(unrecognized error code)";
}
} // namespace

namespace wrap {

const std::error_category &logic_category() noexcept {
  return logic_category_v;
}

const std::error_category &generic_category() noexcept {
  return generic_category_v;
}

const std::error_category &plugin_category() noexcept {
  return plugin_category_v;
}

const std::error_category &action_category() noexcept {
  return action_category_v;
}

void set_default_error_handler(
    void (*func)(const char *, std::error_code) noexcept) noexcept {
  default_error_handler_v = func;
}

void default_error_handler(const char *msg, std::error_code ec) noexcept {
  return default_error_handler_v(msg, std::move(ec));
}

std::error_code make_error_code(logic_errc c) noexcept {
  return std::error_code(static_cast<int>(c), logic_category());
}

std::error_code make_error_code(generic_errc c) noexcept {
  return std::error_code(static_cast<int>(c), generic_category());
}

std::error_code make_error_code(plugin_errc c) noexcept {
  return std::error_code(static_cast<int>(c), plugin_category());
}

std::error_code make_error_code(action_errc c) noexcept {
  return std::error_code(static_cast<int>(c), action_category());
}

void error_code_as_exception(const std::error_code &ec,
                             const error_descriptor &ed) noexcept(false) {
  if (ec.category() == logic_category()) {
    throw logic_error(static_cast<logic_errc>(ec.value()), ed);
  } else if (ec.category() == generic_category()) {
    throw generic_error(static_cast<generic_errc>(ec.value()), ed);
  } else if (ec.category() == plugin_category()) {
    throw plugin_error(static_cast<plugin_errc>(ec.value()), ed);
  } else if (ec.category() == action_category()) {
    throw action_error(static_cast<action_errc>(ec.value()), ed);
  }
  assert(false);
  throw logic_error(logic_errc::invalid_status, null_error_descriptor{});
}

any_error::any_error(std::error_code ec) noexcept : code_(std::move(ec)) {}

namespace detail {

descriptive_error::descriptive_error(std::error_code ec,
                                     const error_descriptor &ed)
    : any_error(std::move(ec)) {
  if (ed)
    data_ = rc_immutable_string{ed.view()};
}

const char *descriptive_error::what() const noexcept {
  if (empty())
    return "(no description available)";
  return data_->c_str();
}

bool descriptive_error::empty() const noexcept {
  return !data_ || data_->empty();
}

} // namespace detail

action_error::action_error(action_errc errc, const error_descriptor &ed)
    : descriptive_error(errc, ed) {}

logic_error::logic_error(logic_errc errc, const error_descriptor &ed)
    : descriptive_error(errc, ed) {}

generic_error::generic_error(generic_errc errc, const error_descriptor &ed)
    : descriptive_error(errc, ed) {}

plugin_error::plugin_error(plugin_errc errc, const error_descriptor &ed)
    : descriptive_error(errc, ed) {}

} // namespace wrap

#pragma warning(disable : 4062)
