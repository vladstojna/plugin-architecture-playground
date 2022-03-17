#include <wrap/action.hpp>
#include <wrap/action_descriptor.hpp>
#include <wrap/error.hpp>
#include <wrap/error_descriptor.hpp>

#include <module_load/module.hpp>
#include <plugin/plugin_interface.h>

#include "status_utils.hpp"

#include <iostream>

namespace {

template <bool ShortVariant>
std::string load_name(const modl::loaded_module &mod,
                      PASMP_action_descriptor_t handle) {
  auto [str, sz] = mod.funcs().action_name(handle, ShortVariant);
  return std::string(str, sz);
}

template <bool ShortVariant>
std::string load_description(const modl::loaded_module &mod,
                             PASMP_action_descriptor_t handle) {
  auto [str, sz] = mod.funcs().action_description(handle, ShortVariant);
  return std::string(str, sz);
}

PASMP_action_descriptor_t create_descriptor(const wrap::action &act,
                                            wrap::error_descriptor &ed) {
  PASMP_action_descriptor_t descr = nullptr;
  ed.clear();
  if (auto status =
          act.get_plugin().get_module().funcs().action_descriptor_create(
              act.get_plugin().get(), act.get(), &descr, &ed))
    status_to_exception(status, ed);
  return descr;
}

PASMP_action_descriptor_t
create_descriptor(const wrap::action &act, std::error_code &ec,
                  wrap::error_descriptor &ed) noexcept {
  PASMP_action_descriptor_t descr = nullptr;
  ed.clear();
  auto status = act.get_plugin().get_module().funcs().action_descriptor_create(
      act.get_plugin().get(), act.get(), &descr, &ed);
  if (ec = wrap::make_error_code(status))
    descr = nullptr;
  return descr;
}

} // namespace

namespace wrap {

struct action_descriptor::wrapper {
  const modl::loaded_module &mod;
  PASMP_action_descriptor_t descr;
  wrapper(const action &, error_descriptor &);
  wrapper(const action &, std::error_code &, error_descriptor &) noexcept;
  ~wrapper();
  wrapper(const wrapper &) = delete;
  wrapper &operator=(const wrapper &) = delete;
};

action_descriptor::action_descriptor(const action &act, error_descriptor &ed)
    : action_descriptor(wrapper{act, ed}) {}

void action_descriptor::load(const action &act, error_descriptor &ed) {
  if (std::error_code ec; !load(act, ec, ed))
    error_code_as_exception(ec, ed);
}

bool action_descriptor::load(const action &act, std::error_code &ec,
                             error_descriptor &ed) {
  ed.clear();
  wrapper w(act, ec, ed);
  if (!ec) {
    name_ = load_name<false>(w.mod, w.descr);
    shortname_ = load_name<true>(w.mod, w.descr);
    descr_ = load_description<false>(w.mod, w.descr);
    shortdescr_ = load_description<true>(w.mod, w.descr);
  }
  return !ec;
}

action_descriptor::action_descriptor(const wrapper &w)
    : basic_descriptor(load_name<false>(w.mod, w.descr),
                       load_name<true>(w.mod, w.descr),
                       load_description<false>(w.mod, w.descr),
                       load_description<true>(w.mod, w.descr)) {}

action_descriptor::wrapper::wrapper(const action &act, error_descriptor &ed)
    : mod(act.get_plugin().get_module()), descr(create_descriptor(act, ed)) {}

action_descriptor::wrapper::wrapper(const action &act, std::error_code &ec,
                                    error_descriptor &ed) noexcept
    : mod(act.get_plugin().get_module()),
      descr(create_descriptor(act, ec, ed)) {}

action_descriptor::wrapper::~wrapper() {
  if (descr)
    if (auto status = mod.funcs().action_descriptor_destroy(descr))
      default_error_handler("Error destroying action descriptor",
                            make_error_code(status));
}

std::ostream &operator<<(std::ostream &os, const action_descriptor &d) {
  os << "name: '" << d.name() << "', name (short): '" << d.short_name()
     << "', description: '" << d.descr() << "', description (short): '"
     << d.short_descr() << "'";
  return os;
}

} // namespace wrap
