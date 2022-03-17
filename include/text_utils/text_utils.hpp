#pragma once

#include <string>
#include <string_view>

namespace text_utils {

static inline std::wstring utf8_to_utf16(std::string_view);
static inline std::string utf16_to_utf8(std::wstring_view);

} // namespace text_utils

#include "text_utils.inl"
