#include "misc.hpp"

#define WIN32_MEAN_AND_LEAN
#include <Windows.h>

#include <ShlObj_core.h>

#include <memory>
#include <mutex>

namespace {

std::filesystem::path persistence_path_impl() {
  struct known_folder_path_deleter {
    void operator()(wchar_t *x) { CoTaskMemFree(x); }
  };

  using known_folder_path_ptr =
      std::unique_ptr<wchar_t, known_folder_path_deleter>;

  PWSTR path = nullptr;
  HRESULT res =
      SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_DEFAULT, nullptr, &path);
  if (res != S_OK)
    throw std::system_error(
        std::error_code{static_cast<int>(res), std::system_category()},
        "Error obtaining persistence path");
  known_folder_path_ptr ptr(path);
  return std::filesystem::path(ptr.get());
}

} // namespace

namespace frontend {

const std::filesystem::path &persistence_path_prefix() {
  static std::filesystem::path retval(persistence_path_impl());
  return retval;
}

} // namespace frontend
