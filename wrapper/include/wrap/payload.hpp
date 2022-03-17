#pragma once

#include <wrap/visibility.hpp>

#include <plugin/plugin_interface.h>

#include <cstdint>
#include <string>
#include <variant>

namespace wrap {

class WRAPPER_DLL_PUBLIC payload {
public:
  explicit payload() noexcept;
  explicit payload(int32_t x) noexcept;
  explicit payload(int64_t x) noexcept;
  explicit payload(uint32_t x) noexcept;
  explicit payload(uint64_t x) noexcept;
  explicit payload(float x) noexcept;
  explicit payload(double x) noexcept;
  explicit payload(std::string x) noexcept;

  operator PASMP_payload_t() const &noexcept;
  operator PASMP_payload_t() && = delete;

private:
  using holder_type = std::variant<std::monostate, int32_t, int64_t, uint32_t,
                                   uint64_t, float, double, std::string>;
  using union_type = PASMP_payload_data_t;
  using tag_type = PASMP_payload_tag_t;

  holder_type holder_;
  tag_type tag_;
  union_type data_;
};

} // namespace wrap
