#include <wrap/error.hpp>
#include <wrap/error_descriptor.hpp>
#include <wrap/plugin_version.hpp>

#include <module_load/module.hpp>
#include <plugin/plugin_interface.h>

#include "status_utils.hpp"

#include <iostream>

namespace {

template <typename Callable>
std::string load_string(Callable getter, PASMP_version_t ver) {
  auto [str, sz] = getter()(ver);
  return std::string(str, sz);
}

PASMP_version_t create_version(const modl::loaded_module &mod,
                               wrap::error_descriptor &ed) {
  PASMP_version_t ver = nullptr;
  ed.clear();
  if (auto status = mod.funcs().version_create(&ver, &ed))
    wrap::status_to_exception(status, ed);
  return ver;
}

} // namespace

namespace wrap {

struct plugin_version::wrapper {
  const modl::loaded_module &mod;
  PASMP_version_t version;
  explicit wrapper(const modl::loaded_module &, error_descriptor &) noexcept;
  ~wrapper();
};

plugin_version::plugin_version(const modl::loaded_module &mod,
                               error_descriptor &ed)
    : plugin_version(mod, wrapper{mod, ed}) {}

plugin_version::plugin_version(const modl::loaded_module &mod, const wrapper &w)
    : major_(mod.funcs().version_major(w.version)),
      minor_(mod.funcs().version_minor(w.version)),
      patch_(mod.funcs().version_patch(w.version)),
      pre_(
          load_string([&mod]() { return mod.funcs().version_pre; }, w.version)),
      build_(load_string([&mod]() { return mod.funcs().version_build; },
                         w.version)) {}

plugin_version::wrapper::wrapper(const modl::loaded_module &mod,
                                 error_descriptor &ed) noexcept
    : mod(mod), version(create_version(mod, ed)) {}

plugin_version::wrapper::~wrapper() {
  if (auto status = mod.funcs().version_destroy(version))
    default_error_handler("Error destroying version", make_error_code(status));
}

std::ostream &operator<<(std::ostream &os, const plugin_version &v) {
  os << v.major() << "." << v.minor() << "." << v.patch();
  if (!v.pre_release().empty())
    os << "-" << v.pre_release();
  if (!v.build().empty())
    os << "+" << v.build();
  return os;
}

} // namespace wrap
