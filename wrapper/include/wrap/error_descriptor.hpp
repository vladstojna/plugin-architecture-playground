#pragma once

#include <wrap/visibility.hpp>

#include <plugin/plugin_interface.h>

#include <string_view>

namespace wrap {

struct WRAPPER_DLL_PUBLIC error_descriptor {
public:
  error_descriptor() noexcept;

  void clear() noexcept;
  bool empty() const noexcept;

  PASMP_error_descriptor_t &raw() &noexcept;

  explicit operator bool() const noexcept { return !empty(); }

  std::string_view view() const noexcept;

  virtual PASMP_error_descriptor_t *operator&() &noexcept { return &raw(); }

protected:
  ~error_descriptor() = default;

private:
  void reset_size() noexcept;
  size_t available() const noexcept;

  virtual char *buffer() noexcept = 0;
  virtual const char *buffer() const noexcept = 0;
  virtual size_t size() const noexcept = 0;

  PASMP_error_descriptor_t descr_;
};

struct WRAPPER_DLL_PUBLIC null_error_descriptor final : error_descriptor {
public:
  PASMP_error_descriptor_t *operator&() &noexcept override;

private:
  const char *buffer() const noexcept override;
  char *buffer() noexcept override;
  size_t size() const noexcept override;
};

template <size_t Size>
struct WRAPPER_DLL_PUBLIC static_error_descriptor final : error_descriptor {
public:
  static_error_descriptor() noexcept { clear(); }

private:
  const char *buffer() const noexcept override { return buffer_; }
  char *buffer() noexcept override { return buffer_; }
  size_t size() const noexcept override { return sizeof(buffer_); }

  char buffer_[Size]{};
};

struct WRAPPER_DLL_PUBLIC dynamic_error_descriptor final : error_descriptor {
public:
  explicit dynamic_error_descriptor(size_t);

private:
  const char *buffer() const noexcept override;
  char *buffer() noexcept override;
  size_t size() const noexcept override;

  std::string data_;
};

} // namespace wrap
