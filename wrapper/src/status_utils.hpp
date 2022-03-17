#pragma once

#include <plugin/plugin_interface.h>

#include <system_error>

namespace wrap {

struct error_descriptor;

std::error_code make_error_code(PASMP_status_t) noexcept;

[[noreturn]] void status_to_exception(PASMP_status_t,
                                      const error_descriptor &) noexcept(false);

void handle_callback_exception() noexcept;

} // namespace wrap
