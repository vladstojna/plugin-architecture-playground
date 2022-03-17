#pragma once

namespace wrap {

template <typename T> class key {
  friend T;
  explicit key() = default;
};

} // namespace wrap
