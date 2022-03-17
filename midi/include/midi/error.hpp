#pragma once

#include <string>
#include <system_error>
#include <type_traits>

namespace midi {
enum class errc : int32_t;
enum class error_cause : int32_t;
} // namespace midi

namespace std {

template <>
struct std::is_error_condition_enum<midi::error_cause> : std::true_type {};

template <> struct std::is_error_code_enum<midi::errc> : std::true_type {};

} // namespace std

namespace midi {

void set_default_error_handler(void (*)(const char *,
                                        std::error_code) noexcept) noexcept;

void default_error_handler(const char *, std::error_code) noexcept;

enum class errc : int32_t {
  callback_exception = 1,
  bad_message,
  attr_bad_interval,
  attr_bad_on_message,
  attr_bad_on_disconnect,
};

enum class error_cause : int32_t {
  bad_device_id = 1,
  driver_error,
  already_in_use,
  invalid_errc,
  other,
};

std::error_code make_error_code(errc) noexcept;

std::error_condition make_error_condition(error_cause) noexcept;

const std::error_category &midi_category() noexcept;
const std::error_category &generic_category() noexcept;

struct midi_error : std::system_error {
  using system_error::system_error;
};

} // namespace midi
