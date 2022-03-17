#pragma once

#include "message.hpp"
#include "passkey.hpp"

#include <functional>

namespace midi {

class midiin;
struct midiin_descriptor_t;

class midiin_attributes {
public:
  using on_message_t = std::function<void(const midiin_descriptor_t &,
                                          message_type, message_data)>;
  using on_disconnect_t = std::function<void(const midiin_descriptor_t &)>;
  using on_error_t = std::function<void(std::exception_ptr)>;
  using interval_t = std::chrono::milliseconds;

  midiin_attributes() noexcept = default;

  midiin_attributes &on_message(on_message_t x) {
    onmsg_ = std::move(x);
    return *this;
  }

  midiin_attributes &on_disconnect(on_disconnect_t x) {
    ondisc_ = std::move(x);
    return *this;
  }

  midiin_attributes &on_error(on_error_t x) {
    onerr_ = std::move(x);
    return *this;
  }

  midiin_attributes &polling_interval(interval_t x) {
    polling_interval_ = std::move(x);
    return *this;
  }

  const on_message_t &on_message(key<midiin>) const noexcept { return onmsg_; }

  const on_error_t &on_error(key<midiin>) const noexcept { return onerr_; }

  const on_disconnect_t &on_disconnect(key<midiin>) const noexcept {
    return ondisc_;
  }

  const interval_t &polling_interval(key<midiin>) const noexcept {
    return polling_interval_;
  }

private:
  on_message_t onmsg_{};
  on_disconnect_t ondisc_{};
  on_error_t onerr_{};
  interval_t polling_interval_{};
};

} // namespace midi
