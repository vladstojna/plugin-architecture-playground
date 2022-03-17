#include <wrap/payload.hpp>

#include <plugin/plugin_interface.h>

#include <cassert>
#include <stdexcept>

namespace wrap {

payload::operator PASMP_payload_t() const &noexcept {
  return {.tag = tag_, .data = &data_};
}

payload::payload() noexcept : tag_{PASMP_PAYLOAD_NONE}, data_{} {}

payload::payload(int32_t x) noexcept
    : holder_(x), tag_{PASMP_PAYLOAD_INT32}, data_{.int32_value =
                                                       std::get<decltype(x)>(
                                                           holder_)} {}

payload::payload(int64_t x) noexcept
    : holder_(x), tag_{PASMP_PAYLOAD_INT64}, data_{.int64_value =
                                                       std::get<decltype(x)>(
                                                           holder_)} {}

payload::payload(uint32_t x) noexcept
    : holder_(x), tag_{PASMP_PAYLOAD_UINT32}, data_{.uint32_value =
                                                        std::get<decltype(x)>(
                                                            holder_)} {}

payload::payload(uint64_t x) noexcept
    : holder_(x), tag_{PASMP_PAYLOAD_UINT64}, data_{.uint64_value =
                                                        std::get<decltype(x)>(
                                                            holder_)} {}

payload::payload(float x) noexcept
    : holder_(x), tag_{PASMP_PAYLOAD_FLOAT}, data_{.float_value =
                                                       std::get<decltype(x)>(
                                                           holder_)} {}

payload::payload(double x) noexcept
    : holder_(x), tag_{PASMP_PAYLOAD_DOUBLE}, data_{.double_value =
                                                        std::get<decltype(x)>(
                                                            holder_)} {}

payload::payload(std::string x) noexcept
    : holder_(std::move(x)), tag_{PASMP_PAYLOAD_STRING}, data_{} {
  const auto &str = std::get<decltype(x)>(holder_);
  data_.string_value = {.data = str.c_str(), .size = str.size()};
}

} // namespace wrap
