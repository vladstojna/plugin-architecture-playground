#include <module_load/module.hpp>
#include <wrap/action.hpp>
#include <wrap/error.hpp>
#include <wrap/error_descriptor.hpp>

#include "status_utils.hpp"

#include <iostream>

namespace {

PASMP_action_t deserialize(const modl::loaded_module &mod, std::string_view str,
                           wrap::error_descriptor &ed) {
  PASMP_action_t retval = nullptr;
  ed.clear();
  if (auto status =
          mod.funcs().action_deserialize(&retval, str.data(), str.size(), &ed))
    wrap::status_to_exception(status, ed);
  return retval;
}

} // namespace

namespace wrap {

size_t action::hash::operator()(const action &x) const noexcept {
  return x.get_plugin().get_module().funcs().action_hash(x.value_);
}

action::action(const plugin &p, PASMP_action_t handle) noexcept
    : plugin_(p), value_(handle) {}

action::action(const plugin &p, PASMP_action_t handle, key<plugin>) noexcept
    : action(p, handle) {}

action::action(const plugin &p, std::string_view data, error_descriptor &ed)
    : action(p, deserialize(p.get_module(), data, ed)) {}

action::action(const action &x) : plugin_(x.plugin_), value_(copy(x.value_)) {}

action::action(action &&x) noexcept
    : plugin_(std::move(x.plugin_)), value_(std::exchange(x.value_, nullptr)) {}

action &action::operator=(const action &x) {
  if (this != &x) {
    plugin_ = x.plugin_;
    value_ = copy(x.value_);
  }
  return *this;
}

action &action::operator=(action &&x) noexcept {
  if (this != &x) {
    plugin_ = std::move(x).plugin_;
    value_ = std::exchange(x.value_, nullptr);
  }
  return *this;
}

action::~action() {
  if (value_)
    if (auto status = get_plugin().get_module().funcs().action_destroy(value_))
      default_error_handler("Error destroying action handle",
                            make_error_code(status));
}

std::string action::serialize(error_descriptor &ed) const {
  std::error_code ec;
  auto retval = serialize(ec, ed);
  if (ec)
    error_code_as_exception(ec, ed);
  return retval;
}

std::string action::serialize(std::error_code &ec, error_descriptor &ed) const {
  std::string retval;
  size_t size = 0;
  auto &function = get_plugin().get_module().funcs().action_serialize;
  // query the size;
  ed.clear();
  if (auto status = function(value_, nullptr, &size, &ed))
    ec = make_error_code(status);
  else {
    // write the contents
    retval.resize(size);
    ed.clear();
    if (auto status = function(value_, retval.data(), &size, &ed)) {
      ec = make_error_code(status);
      retval.clear();
    } else
      ec.clear();
  }
  return retval;
}

PASMP_action_t action::copy(PASMP_action_t x) const {
  // small optimization: only use dynamic memory if size query is larger than
  // static buffer size
  auto [staticbuff, dynbuff] = std::pair<char[32], std::unique_ptr<char[]>>();
  size_t size = 0;
  static_error_descriptor<128> ed;
  auto &function = get_plugin().get_module().funcs().action_serialize;
  // query the size;
  ed.clear();
  if (auto status = function(x, nullptr, &size, &ed))
    wrap::status_to_exception(status, ed);
  char *buffer;
  if (size > sizeof(staticbuff)) {
    dynbuff = std::make_unique<char[]>(size);
    buffer = dynbuff.get();
  } else {
    buffer = staticbuff;
  }
  // write the contents
  ed.clear();
  if (auto status = function(x, buffer, &size, &ed))
    wrap::status_to_exception(status, ed);
  return deserialize(get_plugin().get_module(), {buffer, size}, ed);
}

bool operator==(const action &lhs, const action &rhs) noexcept {
  return lhs.get_plugin() == rhs.get_plugin() &&
         lhs.get_plugin().get_module().funcs().action_equal(lhs.get(),
                                                            rhs.get());
}

std::ostream &operator<<(std::ostream &os, const action &x) {
  static_error_descriptor<128> ed;
  return os << x.serialize(ed);
}

std::string to_string(const action &x) {
  static_error_descriptor<128> ed;
  return x.serialize(ed);
}

} // namespace wrap
