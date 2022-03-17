#include "status_utils.hpp"

#include <wrap/error.hpp>

#pragma warning(error : 4062)

namespace wrap {

std::error_code make_error_code(PASMP_status_t x) noexcept {
  switch (x) {
  case PASMP_SUCCESS:
    return {};
  case PASMP_UNKNOWN:
    return logic_errc::unknown;
  case PASMP_NOT_IMPLEMENTED:
    return generic_errc::not_implemented;
  case PASMP_UNAVAILABLE:
    return plugin_errc::unavailable;
  case PASMP_INVALID_STATUS:
    return logic_errc::invalid_status;
  case PASMP_INVALID_ARGUMENT:
    return logic_errc::invalid_argument;
  case PASMP_ERROR_ALLOC:
    return generic_errc::alloc;
  case PASMP_ERROR_FATAL:
    return logic_errc::fatal;
  case PASMP_ERROR_SYSTEM:
    return generic_errc::system;
  case PASMP_ERROR_TRUNCATED:
    return generic_errc::truncated;
  case PASMP_ERROR_OTHER:
    return generic_errc::other;
  case PASMP_ERROR_PERSISTENCE_PATH:
    return plugin_errc::bad_persistence_path;
  case PASMP_ERROR_PLUGIN:
    return plugin_errc::other;
  case PASMP_ERROR_ACTION_NOENT:
    return plugin_errc::action_not_found;
  case PASMP_ERROR_ACTION_EXEC:
    return action_errc::execution;
  case PASMP_ERROR_ACTION_OTHER:
    return action_errc::other;
  case PASMP_ERROR_ACTION_SERIAL:
    return action_errc::serialization;
  case PASMP_ERROR_PAYLOAD_INVALID:
    return action_errc::invalid_payload;
  }
  return logic_errc::invalid_status;
}

void status_to_exception(PASMP_status_t x,
                         const error_descriptor &ed) noexcept(false) {
  error_code_as_exception(make_error_code(x), ed);
}

void handle_callback_exception() noexcept {
  try {
    throw;
  } catch (const wrap::any_error &e) {
    default_error_handler(e.what(), e.code());
  } catch (const std::exception &e) {
    default_error_handler(
        e.what(), wrap::make_error_code(wrap::logic_errc::callback_exception));
  } catch (...) {
    default_error_handler(
        "Unknown exception",
        wrap::make_error_code(wrap::logic_errc::callback_exception));
  }
}

} // namespace wrap

#pragma warning(disable : 4062)
