#pragma once

#include <wrap/rcstring.hpp>
#include <wrap/visibility.hpp>

#include <plugin/plugin_interface.h>

#include <exception>
#include <optional>
#include <system_error>

namespace wrap {
enum class logic_errc : int32_t;
enum class generic_errc : int32_t;
enum class plugin_errc : int32_t;
enum class action_errc : int32_t;
} // namespace wrap

namespace std {

template <> struct is_error_code_enum<wrap::logic_errc> : std::true_type {};
template <> struct is_error_code_enum<wrap::generic_errc> : std::true_type {};
template <> struct is_error_code_enum<wrap::plugin_errc> : std::true_type {};
template <> struct is_error_code_enum<wrap::action_errc> : std::true_type {};

} // namespace std

namespace wrap {

struct error_descriptor;

enum class logic_errc : int32_t {
  invalid_status = 1,
  invalid_argument,
  callback_exception,
  unknown,
  fatal,
};

enum class generic_errc : int32_t {
  not_implemented = 1,
  alloc,
  system,
  truncated,
  other,
};

enum class plugin_errc : int32_t {
  action_not_found = 1,
  bad_persistence_path,
  unavailable,
  other,
};

enum class action_errc : int32_t {
  execution = 1,
  serialization,
  invalid_payload,
  other,
};

[[noreturn]] WRAPPER_DLL_PUBLIC void
error_code_as_exception(const std::error_code &,
                        const error_descriptor &) noexcept(false);

WRAPPER_DLL_PUBLIC std::error_code make_error_code(logic_errc) noexcept;
WRAPPER_DLL_PUBLIC std::error_code make_error_code(generic_errc) noexcept;
WRAPPER_DLL_PUBLIC std::error_code make_error_code(plugin_errc) noexcept;
WRAPPER_DLL_PUBLIC std::error_code make_error_code(action_errc) noexcept;

WRAPPER_DLL_PUBLIC const std::error_category &logic_category() noexcept;
WRAPPER_DLL_PUBLIC const std::error_category &generic_category() noexcept;
WRAPPER_DLL_PUBLIC const std::error_category &plugin_category() noexcept;
WRAPPER_DLL_PUBLIC const std::error_category &action_category() noexcept;

WRAPPER_DLL_PUBLIC void set_default_error_handler(
    void (*)(const char *, std::error_code) noexcept) noexcept;

WRAPPER_DLL_PUBLIC void default_error_handler(const char *,
                                              std::error_code) noexcept;

struct any_error : std::exception {
  explicit any_error(std::error_code) noexcept;
  const std::error_code &code() const noexcept { return code_; }

private:
  std::error_code code_;
};

namespace detail {

struct descriptive_error : any_error {
public:
  const char *what() const noexcept override;
  bool empty() const noexcept;

protected:
  descriptive_error(std::error_code, const error_descriptor &);

private:
  std::optional<rc_immutable_string> data_;
};

} // namespace detail

struct action_error : detail::descriptive_error {
public:
  action_error(action_errc, const error_descriptor &);
};

struct logic_error : detail::descriptive_error {
public:
  logic_error(logic_errc, const error_descriptor &);
};

struct generic_error : detail::descriptive_error {
public:
  generic_error(generic_errc, const error_descriptor &);
};

struct plugin_error : detail::descriptive_error {
public:
  plugin_error(plugin_errc, const error_descriptor &);
};

} // namespace wrap
