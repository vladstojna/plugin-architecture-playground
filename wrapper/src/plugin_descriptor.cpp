#include <wrap/error.hpp>
#include <wrap/error_descriptor.hpp>
#include <wrap/plugin_descriptor.hpp>

#include <module_load/module.hpp>
#include <plugin/plugin_interface.h>

#include "status_utils.hpp"

#include <iostream>

namespace {

template <bool ShortVariant>
std::string load_name(const modl::loaded_module &mod,
                      PASMP_plugin_descriptor_t handle) {
  auto [str, sz] = mod.funcs().plugin_name(handle, ShortVariant);
  return std::string(str, sz);
}

template <bool ShortVariant>
std::string load_description(const modl::loaded_module &mod,
                             PASMP_plugin_descriptor_t handle) {

  auto [str, sz] = mod.funcs().plugin_description(handle, ShortVariant);
  return std::string(str, sz);
}

PASMP_plugin_descriptor_t create_descriptor(const modl::loaded_module &mod,
                                            wrap::error_descriptor &ed) {
  PASMP_plugin_descriptor_t descr = nullptr;
  ed.clear();
  if (auto status = mod.funcs().plugin_descriptor_create(&descr, &ed))
    wrap::status_to_exception(status, ed);
  return descr;
}

} // namespace

namespace wrap {

struct plugin_descriptor::wrapper {
  const modl::loaded_module &mod;
  PASMP_plugin_descriptor_t descr;
  explicit wrapper(const modl::loaded_module &, error_descriptor &);
  ~wrapper();
  wrapper(const wrapper &) = delete;
  wrapper &operator=(const wrapper &) = delete;
};

plugin_descriptor::plugin_descriptor(const modl::loaded_module &mod,
                                     error_descriptor &ed)
    : plugin_descriptor(wrapper{mod, ed}) {}

plugin_descriptor::plugin_descriptor(const wrapper &w)
    : basic_descriptor(load_name<false>(w.mod, w.descr),
                       load_name<true>(w.mod, w.descr),
                       load_description<false>(w.mod, w.descr),
                       load_description<true>(w.mod, w.descr)) {}

plugin_descriptor::wrapper::wrapper(const modl::loaded_module &mod,
                                    error_descriptor &ed)
    : mod(mod), descr(create_descriptor(mod, ed)) {}

plugin_descriptor::wrapper::~wrapper() {
  if (auto status = mod.funcs().plugin_descriptor_destroy(descr))
    default_error_handler("Error destroying plugin descriptor",
                          make_error_code(status));
}

std::ostream &operator<<(std::ostream &os, const plugin_descriptor &d) {
  os << "name: '" << d.name() << "', name (short): '" << d.short_name()
     << "', description: '" << d.descr() << "', description (short): '"
     << d.short_descr() << "'";
  return os;
}

} // namespace wrap
