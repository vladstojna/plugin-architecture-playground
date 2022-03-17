#include "text_utils.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace text_utils {

std::wstring utf8_to_utf16(std::string_view str) {
  std::wstring returnValue;

  int wideCharSize =
      MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, str.data(),
                          static_cast<int>(str.size()), nullptr, 0);
  if (wideCharSize == 0)
    return returnValue;
  returnValue.resize(wideCharSize);
  wideCharSize = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, str.data(),
                                     static_cast<int>(str.size()),
                                     &returnValue[0], wideCharSize);
  if (wideCharSize == 0)
    returnValue.resize(0);
  return returnValue;
}

std::string utf16_to_utf8(std::wstring_view str) {
  std::string returnValue;

  int mbCharSize = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
                                       str.data(), static_cast<int>(str.size()),
                                       nullptr, 0, nullptr, nullptr);
  if (mbCharSize == 0) {
    return returnValue;
  }
  returnValue.resize(mbCharSize);
  mbCharSize = WideCharToMultiByte(
      CP_UTF8, WC_ERR_INVALID_CHARS, str.data(), static_cast<int>(str.size()),
      &returnValue[0], mbCharSize, nullptr, nullptr);
  if (mbCharSize == 0)
    returnValue.resize(0);
  return returnValue;
}

} // namespace text_utils
