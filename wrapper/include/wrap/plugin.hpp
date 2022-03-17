#pragma once

#include <wrap/action_event.hpp>
#include <wrap/visibility.hpp>

#include <module_load/modulefwd.hpp>
#include <plugin/plugin_interface.h>

#include <memory>
#include <system_error>
#include <vector>

namespace wrap {

class action;
class plugin_attributes;
struct error_descriptor;

class WRAPPER_DLL_PUBLIC plugin {
public:
  plugin(const modl::loaded_module &, plugin_attributes, error_descriptor &);

  std::vector<action> actions(error_descriptor &) const;
  std::vector<action> actions(std::error_code &, error_descriptor &) const;

  PASMP_plugin_t get() const noexcept;

  const modl::loaded_module &get_module() const noexcept;

  const plugin_attributes &attributes() const noexcept;

private:
  struct impl;

  explicit plugin(const impl &);

  std::shared_ptr<const impl> impl_;
};

WRAPPER_DLL_PUBLIC bool operator==(const plugin &, const plugin &);

} // namespace wrap
