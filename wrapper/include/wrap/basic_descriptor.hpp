#pragma once

#include <wrap/visibility.hpp>

#include <string>

namespace wrap {

class WRAPPER_DLL_PUBLIC basic_descriptor {
public:
  const std::string &name() const noexcept { return name_; };
  const std::string &descr() const noexcept { return descr_; };
  const std::string &short_name() const noexcept { return shortname_; };
  const std::string &short_descr() const noexcept { return shortdescr_; };

protected:
  basic_descriptor() = default;

  basic_descriptor(std::string &&name, std::string &&sname, std::string &&descr,
                   std::string &&sdescr)
      : name_(std::move(name)), shortname_(std::move(sname)),
        descr_(std::move(descr)), shortdescr_(std::move(sdescr)) {}

  basic_descriptor(const basic_descriptor &) = default;
  basic_descriptor(basic_descriptor &&) = default;

  basic_descriptor &operator=(const basic_descriptor &) = default;
  basic_descriptor &operator=(basic_descriptor &&) = default;

  ~basic_descriptor() = default;

  std::string name_;
  std::string shortname_;
  std::string descr_;
  std::string shortdescr_;
};

} // namespace wrap
