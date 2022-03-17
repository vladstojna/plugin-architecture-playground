#pragma once

#include <wrap/visibility.hpp>

#include <module_load/modulefwd.hpp>

#include <iosfwd>
#include <string>

namespace wrap {

struct error_descriptor;

class WRAPPER_DLL_PUBLIC plugin_version {
public:
  explicit plugin_version(const modl::loaded_module &, error_descriptor &);

  uint16_t major() const noexcept { return major_; }
  uint16_t minor() const noexcept { return minor_; }
  uint16_t patch() const noexcept { return patch_; }
  const std::string &pre_release() const noexcept { return pre_; }
  const std::string &build() const noexcept { return build_; }

private:
  struct wrapper;
  plugin_version(const modl::loaded_module &, const wrapper &);

  uint16_t major_;
  uint16_t minor_;
  uint16_t patch_;
  std::string pre_;
  std::string build_;
};

WRAPPER_DLL_PUBLIC std::ostream &operator<<(std::ostream &,
                                            const plugin_version &);

} // namespace wrap
