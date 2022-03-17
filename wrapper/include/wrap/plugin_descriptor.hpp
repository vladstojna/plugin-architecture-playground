#pragma once

#include <wrap/basic_descriptor.hpp>
#include <wrap/visibility.hpp>

#include <module_load/modulefwd.hpp>

#include <iosfwd>

namespace wrap {

struct error_descriptor;

class WRAPPER_DLL_PUBLIC plugin_descriptor final : public basic_descriptor {
public:
  plugin_descriptor(const modl::loaded_module &, error_descriptor &);

private:
  struct wrapper;
  explicit plugin_descriptor(const wrapper &);
};

WRAPPER_DLL_PUBLIC std::ostream &operator<<(std::ostream &,
                                            const plugin_descriptor &);

} // namespace wrap
