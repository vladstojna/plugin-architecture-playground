#pragma once

#include <wrap/passkey.hpp>
#include <wrap/plugin.hpp>
#include <wrap/visibility.hpp>

#include <plugin/plugin_interface.h>

#include <iosfwd>
#include <string>
#include <string_view>

namespace wrap {

struct error_descriptor;

class WRAPPER_DLL_PUBLIC action {
public:
  struct hash {
    size_t operator()(const action &) const noexcept;
  };

  action(const plugin &, PASMP_action_t, key<plugin>) noexcept;
  action(const plugin &, std::string_view, error_descriptor &);

  action(const action &);
  action(action &&) noexcept;

  action &operator=(const action &);
  action &operator=(action &&) noexcept;

  ~action();

  PASMP_action_t get() const noexcept { return value_; }
  const plugin &get_plugin() const noexcept { return plugin_; }

  std::string serialize(error_descriptor &) const;
  std::string serialize(std::error_code &, error_descriptor &) const;

private:
  action(const plugin &, PASMP_action_t) noexcept;

  PASMP_action_t copy(PASMP_action_t) const;

  plugin plugin_;
  PASMP_action_t value_;
};

WRAPPER_DLL_PUBLIC bool operator==(const action &, const action &) noexcept;

WRAPPER_DLL_PUBLIC std::ostream &operator<<(std::ostream &, const action &);

WRAPPER_DLL_PUBLIC std::string to_string(const action &);

} // namespace wrap
