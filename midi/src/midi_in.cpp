#include <midi/error.hpp>
#include <midi/message.hpp>
#include <midi/midi_in.hpp>
#include <midi/midiin_attributes.hpp>

#include <text_utils/text_utils.hpp>

#include "wincommon.h"

#include <cassert>
#include <condition_variable>
#include <thread>

namespace {

void handle_callback_exception() noexcept {
  using namespace midi;
  try {
    throw;
  } catch (const midi::midi_error &e) {
    default_error_handler(e.what(), errc::callback_exception);
  } catch (const std::exception &e) {
    default_error_handler(e.what(), errc::callback_exception);
  } catch (...) {
    default_error_handler("Unknown exception", errc::callback_exception);
  }
}

} // namespace

namespace midi {

midiin_descriptor_t::midiin_descriptor_t(uint32_t p) : port(p) {
  MIDIINCAPSW caps{};
  if (auto res = midiInGetDevCapsW(port, &caps, sizeof(caps));
      MMSYSERR_NOERROR != res)
    throw midi_error(res, midi_category(),
                     "Error getting MIDI input device descriptor");
  manufacturer_id = caps.wMid;
  product_id = caps.wPid;
  driver_version = {.major = std::byte{(caps.vDriverVersion >> 8) && 0xff},
                    .minor = std::byte{caps.vDriverVersion && 0xff}};
  name = text_utils::utf16_to_utf8(caps.szPname);
}

uint32_t midiin_port_count() noexcept { return midiInGetNumDevs(); }

std::vector<std::pair<uint32_t, midiin_descriptor_t>> midiin_descriptors() {
  decltype(midiin_descriptors()) descrs;
  auto count = midiin_port_count();
  if (count) {
    descrs.reserve(count);
    for (decltype(count) d = 0; d < count; d++)
      descrs.emplace_back(d, d);
  }
  return descrs;
}

std::vector<std::pair<uint32_t, midiin_descriptor_t>>
midiin_descriptors(std::error_code &ec) {
  try {
    auto vec = midiin_descriptors();
    ec.clear();
    return vec;
  } catch (const midi_error &e) {
    ec = e.code();
    return {};
  }
}

bool operator==(const driver_version_t &lhs,
                const driver_version_t &rhs) noexcept {
  return lhs.major == rhs.major && lhs.minor == rhs.minor;
}

bool operator==(const midiin_descriptor_t &lhs,
                const midiin_descriptor_t &rhs) noexcept {
  return lhs.port == rhs.port && lhs.driver_version == rhs.driver_version &&
         lhs.manufacturer_id == rhs.manufacturer_id &&
         lhs.product_id == rhs.product_id && lhs.name == rhs.name;
}

struct midiin::impl {
  using handle_t = HMIDIIN;

  struct heartbeat {
  public:
    explicit heartbeat(midiin::impl *x)
        : impl_(x), checker_([this](std::stop_token stoken) {
            key<midiin> k;
            std::mutex mux;
            const auto &descr = impl_->descr;
            while (true) {
              std::unique_lock lk(mux);
              std::condition_variable_any{}.wait_for(
                  lk, stoken, impl_->attr.polling_interval(k),
                  []() { return false; });
              if (stoken.stop_requested())
                break;
              try {
                auto devs = midiin_descriptors();
                auto it = std::find_if(
                    devs.begin(), devs.end(),
                    [&descr](const auto &x) { return descr == x.second; });
                if (it == devs.end()) {
                  impl_->attr.on_disconnect(k)(descr);
                  break;
                }
              } catch (...) {
                if (impl_->attr.on_error(k))
                  try {
                    impl_->attr.on_error(k)(std::current_exception());
                  } catch (...) {
                    handle_callback_exception();
                  }
                else
                  handle_callback_exception();
                break;
              }
            }
          }) {}

  private:
    midiin::impl *impl_;
    std::jthread checker_;
  };

  midiin_descriptor_t descr;
  midiin_attributes attr;
  handle_t handle;
  heartbeat hb;

  impl(midiin_descriptor_t &&d, start_behaviour sb, midiin_attributes &&a)
      : descr(std::move(d)), attr(std::move(a)),
        handle(create_midiin_handle(descr.port)), hb(this) {
    key<midiin> k;
    if (attr.polling_interval(k).count() <= 0)
      throw midi_error(errc::attr_bad_interval, "Attribute error");
    if (!attr.on_message(k))
      throw midi_error(errc::attr_bad_on_message, "Attribute error");
    if (!attr.on_disconnect(k))
      throw midi_error(errc::attr_bad_on_disconnect, "Attribute error");
    if (sb == start_behaviour::start)
      if (std::error_code ec; !start(ec))
        throw midi_error(ec, "Error starting MIDI input device");
  }

  bool start(std::error_code &ec) noexcept {
    if (auto res = midiInStart(handle); MMSYSERR_NOERROR != res)
      ec = std::error_code{int(res), midi_category()};
    else
      ec.clear();
    return !ec;
  }

  bool stop(std::error_code &ec) noexcept {
    if (auto res = midiInStop(handle); MMSYSERR_NOERROR != res)
      ec = std::error_code{int(res), midi_category()};
    else
      ec.clear();
    return !ec;
  }

  ~impl() { destroy(); }

  void destroy() noexcept {
    if (std::error_code ec; !stop(ec))
      default_error_handler("Error stopping MIDI input device", ec);
    if (auto res = midiInClose(handle); MMSYSERR_NOERROR != res)
      default_error_handler("Error closing MIDI input device",
                            std::error_code{int(res), midi_category()});
  }

  handle_t create_midiin_handle(uint32_t port, std::error_code &ec) noexcept {
    handle_t handle = nullptr;
    auto res =
        midiInOpen(&handle, port, reinterpret_cast<DWORD_PTR>(&wrap_callback),
                   reinterpret_cast<DWORD_PTR>(this), CALLBACK_FUNCTION);
    if (MMSYSERR_NOERROR != res) {
      ec = std::error_code{int(res), midi_category()};
      handle = nullptr;
    } else
      ec.clear();
    return handle;
  }

  handle_t create_midiin_handle(uint32_t port) {
    handle_t handle;
    if (std::error_code ec; !(handle = create_midiin_handle(port, ec)))
      throw midi_error(ec, "Error opening MIDI in device");
    return handle;
  }

  static void CALLBACK wrap_callback(HMIDIIN hMidiIn, UINT wMsg,
                                     DWORD_PTR dwInstance, DWORD_PTR dwParam1,
                                     DWORD_PTR dwParam2) {
    auto convert_msg = [](DWORD_PTR t, DWORD_PTR x) {
      return message_data{
          .timestamp =
              message_data::time_point{message_data::time_point::duration{t}},
          .status = make_message_status(std::byte(x & 0xff)),
          .data = {std::byte((x >> 8) & 0xff), std::byte((x >> 16) & 0xff)}};
    };

    key<midiin> k;
    const auto &imp = *reinterpret_cast<const impl *>(dwInstance);
    try {
      message_type msg_type = message_type::unknown;
      message_data msg;
      switch (wMsg) {
      case MIM_OPEN:
        msg_type = message_type::open;
        break;
      case MIM_CLOSE:
        msg_type = message_type::close;
        break;
      case MIM_ERROR:
        msg_type = message_type::error;
        msg = convert_msg(dwParam2, dwParam1);
        break;
      case MIM_DATA:
        msg_type = message_type::data;
        msg = convert_msg(dwParam2, dwParam1);
        break;
      case MIM_LONGDATA:
        msg_type = message_type::sys_excl_done;
        break;
      case MIM_LONGERROR:
        msg_type = message_type::sys_excl_error;
        break;
      case MIM_MOREDATA:
        msg_type = message_type::other;
        msg = convert_msg(dwParam2, dwParam1);
        break;
      }
      imp.attr.on_message(k)(imp.descr, msg_type, msg);
    } catch (...) {
      if (imp.attr.on_error(k)) {
        try {
          imp.attr.on_error(k)(std::current_exception());
        } catch (...) {
          handle_callback_exception();
        }
      } else {
        handle_callback_exception();
      }
    }
  }
}; // namespace midi

midiin::midiin(midiin_descriptor_t descr, midiin_attributes attr,
               start_behaviour sb)
    : impl_(std::make_unique<impl>(std::move(descr), sb, std::move(attr))) {}

midiin::midiin(midiin &&) noexcept = default;
midiin &midiin::operator=(midiin &&) noexcept = default;
midiin::~midiin() = default;

const midiin_descriptor_t &midiin::descriptor() const noexcept {
  return impl_->descr;
}

void midiin::start() {
  if (std::error_code ec; !start(ec))
    throw midi_error(ec, "Error starting MIDI input device");
}

bool midiin::start(std::error_code &ec) noexcept { return impl_->start(ec); }

void midiin::stop() {
  if (std::error_code ec; !stop(ec))
    throw midi_error(ec, "Error stopping MIDI input device");
}

bool midiin::stop(std::error_code &ec) noexcept { return impl_->stop(ec); }

} // namespace midi
