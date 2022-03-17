#pragma once

#include <wrap/visibility.hpp>

#include <module_load/module.hpp>

namespace wrap {

class WRAPPER_DLL_PUBLIC plugin_object {
public:
  explicit plugin_object(const modl::loaded_module &mod) noexcept : mod_(mod) {}

  const modl::loaded_module &get_module() const noexcept { return mod_; }

protected:
  ~plugin_object() = default;

private:
  modl::loaded_module mod_;
};

} // namespace wrap
