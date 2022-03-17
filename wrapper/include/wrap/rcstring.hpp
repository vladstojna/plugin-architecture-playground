#pragma once

#include <wrap/visibility.hpp>

#include <memory>
#include <string>

namespace wrap {

class WRAPPER_DLL_PUBLIC rc_immutable_string {
public:
  using str_type = std::string;
  using value_type = str_type::value_type;
  using size_type = str_type::size_type;
  using const_iterator = str_type::const_iterator;
  using const_reverse_iterator = str_type::const_reverse_iterator;

  explicit rc_immutable_string(std::string_view);

  operator const str_type &() const noexcept;

  const value_type *c_str() const noexcept;
  size_type size() const noexcept;
  bool empty() const noexcept;

  const_iterator begin() const noexcept;
  const_iterator cbegin() const noexcept;
  const_iterator end() const noexcept;
  const_iterator cend() const noexcept;

  const_reverse_iterator rbegin() const noexcept;
  const_reverse_iterator rend() const noexcept;
  const_reverse_iterator crbegin() const noexcept;
  const_reverse_iterator crend() const noexcept;

private:
  std::shared_ptr<const str_type> data_;
};

} // namespace wrap
