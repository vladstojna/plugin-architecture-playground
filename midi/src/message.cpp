#include <midi/message.hpp>

namespace {

struct channel_voice_category_t : midi::message_category {
  const char *name() const noexcept override { return "Channel Voice"; }

  const char *descr(std::byte x) const noexcept override {
    auto val = static_cast<midi::channel_voice>(x & std::byte{0xf0});
    switch (val) {
    case midi::channel_voice::note_off:
      return "Note Off";
    case midi::channel_voice::note_on:
      return "Note On";
    case midi::channel_voice::polyphonic_key_pressure:
      return "Polyphonic Key Pressure";
    case midi::channel_voice::control_change:
      return "Control Change";
    case midi::channel_voice::program_change:
      return "Program Change";
    case midi::channel_voice::channel_pressure:
      return "Channel Pressure";
    case midi::channel_voice::pitch_bend_change:
      return "Pitch Bend Change";
    }
    return "(unrecognized status)";
  }
};

struct system_common_category_t : midi::message_category {
  const char *name() const noexcept override { return "System Common"; }

  const char *descr(std::byte x) const noexcept override {
    auto val = static_cast<midi::system_common>(x);
    switch (val) {
    case midi::system_common::system_exclusive:
      return "System Exclusive Start";
    case midi::system_common::time_code_quarter_frame:
      return "Time Code Quarter Frame";
    case midi::system_common::song_position_pointer:
      return "Song Position Pointer";
    case midi::system_common::song_select:
      return "Song Select";
    case midi::system_common::tune_request:
      return "Tune Request";
    case midi::system_common::end_of_exclusive:
      return "System Exclusive End";
    }
    return "(unrecognized status)";
  }
};

struct system_realtime_category_t : midi::message_category {
  const char *name() const noexcept override { return "System Real-Time"; }

  const char *descr(std::byte x) const noexcept override {
    auto val = static_cast<midi::system_realtime>(x);
    switch (val) {
    case midi::system_realtime::timing_clock:
      return "Timing Clock";
    case midi::system_realtime::start:
      return "Start";
    case midi::system_realtime::cont:
      return "Continue";
    case midi::system_realtime::stop:
      return "Stop";
    case midi::system_realtime::active_sensing:
      return "Active Sensing";
    case midi::system_realtime::reset:
      return "Reset";
    }
    return "(unrecognized status)";
  }
};

const channel_voice_category_t channel_voice_category_v;
const system_common_category_t system_common_category_v;
const system_realtime_category_t system_realtime_category_v;

} // namespace

namespace midi {

const message_category &channel_voice_cat() noexcept {
  return channel_voice_category_v;
}

const message_category &system_common_cat() noexcept {
  return system_common_category_v;
}

const message_category &system_realtime_cat() noexcept {
  return system_realtime_category_v;
}

message_status make_message_status(std::byte x) noexcept {
  if (channel_voice cv{std::to_integer<uint8_t>(x & std::byte(0xf0))};
      cv >= channel_voice::note_off && cv <= channel_voice::pitch_bend_change)
    return make_message_status(channel_voice{std::to_integer<uint8_t>(x)});
  if (system_common sc{std::to_integer<uint8_t>(x)};
      sc >= system_common::system_exclusive &&
      sc <= system_common::end_of_exclusive)
    return make_message_status(sc);
  if (system_realtime sr{std::to_integer<uint8_t>(x)};
      sr >= system_realtime::timing_clock && sr <= system_realtime::reset)
    return make_message_status(sr);
  return message_status{};
}

message_status make_message_status(channel_voice x) noexcept {
  return message_status(std::byte{static_cast<uint8_t>(x)},
                        channel_voice_cat());
}

message_status make_message_status(system_common x) noexcept {
  return message_status(std::byte{static_cast<uint8_t>(x)},
                        system_common_cat());
}

message_status make_message_status(system_realtime x) noexcept {
  return message_status(std::byte{static_cast<uint8_t>(x)},
                        system_realtime_cat());
}

bool operator==(const message_status &lhs, const message_status &rhs) noexcept {
  if (lhs.category() == rhs.category()) {
    if (lhs.category() == channel_voice_cat())
      return (lhs.value() & std::byte(0xf0)) == (rhs.value() & std::byte(0xf0));
    return lhs.value() == rhs.value();
  }
  return false;
}

std::strong_ordering operator<=>(const message_status &lhs,
                                 const message_status &rhs) noexcept {
  if (auto res = lhs.category() <=> rhs.category(); res != 0)
    return res;
  if (lhs.category() == channel_voice_cat())
    return (lhs.value() & std::byte(0xf0)) <=> (rhs.value() & std::byte(0xf0));
  return lhs.value() <=> rhs.value();
}

} // namespace midi
