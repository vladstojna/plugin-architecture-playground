#include <midi/error.hpp>

#include <text_utils/text_utils.hpp>

#include "wincommon.h"

#include <iostream>

#pragma warning(error : 4062)

namespace {

std::ostream &operator<<(std::ostream &os, std::error_condition errcond) {
  os << errcond.category().name() << ":" << errcond.value();
  return os;
}

auto default_error_handler_v =
    +[](const char *msg, std::error_code ec) noexcept {
      std::cerr << msg << " (" << ec << ":" << ec.default_error_condition()
                << ")" << std::endl;
    };

void get_error_string(MMRESULT res, wchar_t *data, UINT size) {
  auto status = midiInGetErrorTextW(res, data, size);
  if (MMSYSERR_NOMEM == status)
    throw std::bad_alloc{};
  else if (MMSYSERR_BADERRNUM == status)
    get_error_string(MMSYSERR_BADERRNUM, data, size);
  else if (MMSYSERR_NOERROR != status)
    throw std::logic_error("Error getting error string");
}

std::string get_error_string(MMRESULT res) {
  std::wstring wide;
  wide.resize(MAXERRORLENGTH);
  get_error_string(res, wide.data(), static_cast<UINT>(wide.size()));
  return text_utils::utf16_to_utf8(wide);
}

struct midi_category_t : std::error_category {
  const char *name() const noexcept override;
  std::string message(int) const override;
  std::error_condition default_error_condition(int) const noexcept override;
};

struct generic_category_t : std::error_category {
  const char *name() const noexcept override;
  std::string message(int) const override;
};

struct error_cause_category_t : std::error_category {
  const char *name() const noexcept override;
  std::string message(int) const override;
  bool equivalent(const std::error_code &, int) const noexcept override;
};

const midi_category_t midi_category_v;
const generic_category_t generic_category_v;
const error_cause_category_t error_cause_category_v;

const char *midi_category_t::name() const noexcept { return "midi"; }

std::string midi_category_t::message(int ev) const {
  return get_error_string(static_cast<MMRESULT>(ev));
}

const char *generic_category_t::name() const noexcept { return "midi-generic"; }

std::string generic_category_t::message(int ev) const {
  using midi::errc;
  switch (static_cast<errc>(ev)) {
  case errc::callback_exception:
    return "exception in user-provided callback";
  case errc::bad_message:
    return "bad MIDI message";
  case errc::attr_bad_interval:
    return "bad polling interval in attributes";
  case errc::attr_bad_on_message:
    return "bad message callback in attributes";
  case errc::attr_bad_on_disconnect:
    return "bad disconnect callback in attributes";
  }
  return "(unrecognized error code)";
}

const char *error_cause_category_t::name() const noexcept {
  return "error-cause";
}

std::string error_cause_category_t::message(int ev) const {
  using midi::error_cause;
  switch (static_cast<error_cause>(ev)) {
  case error_cause::bad_device_id:
    return "bad device ID";
  case error_cause::driver_error:
    return "driver error";
  case error_cause::already_in_use:
    return "device already in use";
  case error_cause::invalid_errc:
    return "invalid error code";
  case error_cause::other:
    return "other error";
  }
  return "(unrecognized error condition)";
}

std::error_condition
midi_category_t::default_error_condition(int ev) const noexcept {
  using midi::error_cause;
  switch (ev) {
  case MMSYSERR_BADDEVICEID:
    return error_cause::bad_device_id;
  case MMSYSERR_NOTENABLED:
  case MMSYSERR_NODRIVER:
  case MMSYSERR_NODRIVERCB:
    return error_cause::driver_error;
  case MMSYSERR_ALLOCATED:
    return error_cause::already_in_use;
  case MMSYSERR_BADERRNUM:
    return error_cause::invalid_errc;
  }
  if (ev >= MMSYSERR_ERROR && ev <= MMSYSERR_LASTERROR)
    return error_cause::other;
  return error_cause::invalid_errc;
}

bool error_cause_category_t::equivalent(const std::error_code &ec,
                                        int cond) const noexcept {
  using midi::error_cause;
  using midi::midi_category;
  if (ec.category() != midi_category())
    return false;
  switch (static_cast<error_cause>(cond)) {
  case error_cause::bad_device_id:
    return ec.value() == MMSYSERR_BADDEVICEID;
  case error_cause::driver_error:
    return ec.value() == MMSYSERR_NOTENABLED ||
           ec.value() == MMSYSERR_NODRIVER || ec.value() == MMSYSERR_NODRIVERCB;
  case error_cause::already_in_use:
    return ec.value() == MMSYSERR_ALLOCATED;
  case error_cause::invalid_errc:
    return ec.value() == MMSYSERR_BADERRNUM;
  case error_cause::other:
    return ec.value() >= MMSYSERR_ERROR && ec.value() <= MMSYSERR_LASTERROR;
  }
  return false;
}

} // namespace

namespace midi {

void set_default_error_handler(
    void (*func)(const char *, std::error_code) noexcept) noexcept {
  default_error_handler_v = func;
}

void default_error_handler(const char *msg, std::error_code ec) noexcept {
  default_error_handler_v(msg, std::move(ec));
}

const std::error_category &midi_category() noexcept { return midi_category_v; }

std::error_code midi::make_error_code(errc ec) noexcept {
  return std::error_code(static_cast<int>(ec), generic_category_v);
}

std::error_condition make_error_condition(error_cause ec) noexcept {
  return std::error_condition(static_cast<int>(ec), error_cause_category_v);
}

} // namespace midi

#pragma warning(disable : 4062)
