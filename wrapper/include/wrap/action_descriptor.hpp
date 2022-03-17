#pragma once

#include <wrap/basic_descriptor.hpp>
#include <wrap/visibility.hpp>

#include <iosfwd>
#include <system_error>

namespace wrap {

class action;
struct error_descriptor;

class WRAPPER_DLL_PUBLIC action_descriptor final : public basic_descriptor {
public:
  action_descriptor() = default;
  action_descriptor(const action &, error_descriptor &);

  void load(const action &, error_descriptor &);
  bool load(const action &, std::error_code &, error_descriptor &);

private:
  struct wrapper;
  explicit action_descriptor(const wrapper &);
};

WRAPPER_DLL_PUBLIC std::ostream &operator<<(std::ostream &,
                                            const action_descriptor &);

} // namespace wrap
