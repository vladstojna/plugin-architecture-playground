#pragma once

#include <array>
#include <chrono>
#include <compare>
#include <cstddef>
#include <cstdint>

namespace midi {

enum class message_type : uint32_t {
  open,
  close,
  data,
  error,
  sys_excl_done,
  sys_excl_error,
  other,
  unknown,
};

enum class channel_voice : uint8_t {
  note_off = 0b1000'0000,
  note_on = 0b1001'0000,
  polyphonic_key_pressure = 0b1010'0000,
  control_change = 0b1011'0000,
  program_change = 0b1100'0000,
  channel_pressure = 0b1101'0000,
  pitch_bend_change = 0b1110'0000,
};

enum class system_common : uint8_t {
  system_exclusive = 0b1111'0000,
  time_code_quarter_frame = 0b1111'0001,
  song_position_pointer = 0b1111'0010,
  song_select = 0b1111'0011,
  tune_request = 0b1111'0110,
  end_of_exclusive = 0b1111'0111,
};

enum class system_realtime : uint8_t {
  timing_clock = 0b1111'1000,
  start = 0b1111'1010,
  cont = 0b1111'1011,
  stop = 0b1111'1100,
  active_sensing = 0b1111'1110,
  reset = 0b1111'1111,
};

struct message_category {
  virtual const char *name() const noexcept = 0;
  virtual const char *descr(std::byte) const noexcept = 0;

  bool operator==(const message_category &rhs) const noexcept {
    return this == &rhs;
  }

  std::strong_ordering operator<=>(const message_category &rhs) const noexcept {
    return std::compare_three_way{}(this, &rhs);
  }
};

const message_category &channel_voice_cat() noexcept;
const message_category &system_common_cat() noexcept;
const message_category &system_realtime_cat() noexcept;

template <typename> struct is_message_enum : std::false_type {};
template <> struct is_message_enum<channel_voice> : std::true_type {};
template <> struct is_message_enum<system_common> : std::true_type {};
template <> struct is_message_enum<system_realtime> : std::true_type {};

class message_status {
public:
  message_status() noexcept : status_{}, cat_{&channel_voice_cat()} {}

  message_status(std::byte val, const message_category &cat) noexcept
      : status_(val), cat_(&cat) {}

  template <typename MEnum,
            typename = std::enable_if_t<is_message_enum<MEnum>::value>>
  message_status(MEnum x) noexcept : status_{}, cat_(nullptr) {
    *this = make_message_status(x);
  }

  std::byte value() const noexcept { return status_; }

  std::byte channel() const noexcept { return status_ & std::byte{0xf}; }

  const message_category &category() const noexcept { return *cat_; }

  const char *descr() const noexcept { return cat_->descr(value()); }

  explicit operator bool() const noexcept { return !bool(status_); }

private:
  std::byte status_;
  const message_category *cat_;
};

message_status make_message_status(std::byte) noexcept;
message_status make_message_status(channel_voice) noexcept;
message_status make_message_status(system_common) noexcept;
message_status make_message_status(system_realtime) noexcept;

bool operator==(const message_status &, const message_status &) noexcept;

std::strong_ordering operator<=>(const message_status &,
                                 const message_status &) noexcept;

struct message_data {
  using time_point = std::chrono::time_point<std::chrono::steady_clock,
                                             std::chrono::milliseconds>;
  time_point timestamp;
  message_status status;
  std::byte data[2];
};

} // namespace midi
