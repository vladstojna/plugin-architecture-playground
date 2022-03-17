#pragma once

namespace midi {

template <typename T> class key {
  friend T;
  explicit key() = default;
};

} // namespace midi
