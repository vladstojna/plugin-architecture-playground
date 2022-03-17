#pragma once

#include <cstdint>

namespace wrap {

enum class action_event : uint32_t {
  add,
  modify,
  remove,
};

}
