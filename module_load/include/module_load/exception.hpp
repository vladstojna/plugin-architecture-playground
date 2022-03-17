#pragma once

#include <module_load/version.hpp>

#include <exception>
#include <memory>
#include <string_view>
#include <system_error>

namespace modl {

class module_error : public std::exception {
public:
  using exception::exception;
};

class module_incompatible : public module_error {
public:
  module_incompatible(library_version current, library_version found) noexcept;

  library_version current() const noexcept;
  library_version found() const noexcept;

  const char *what() const noexcept override;

private:
  library_version current_;
  library_version found_;
};

class function_load_error : public module_error {
public:
  function_load_error(std::error_code, std::string_view);

  const char *name() const noexcept;
  std::error_code code() const noexcept;

  const char *what() const noexcept override;

private:
  struct context;
  std::shared_ptr<const context> ctx_;
};

} // namespace modl
