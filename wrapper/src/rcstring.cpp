#include <wrap/rcstring.hpp>

namespace wrap {

rc_immutable_string::rc_immutable_string(std::string_view x)
    : data_(std::make_shared<str_type>(x)) {}

rc_immutable_string::operator const str_type &() const noexcept {
  return *data_;
}

bool rc_immutable_string::empty() const noexcept { return data_->empty(); }

const rc_immutable_string::value_type *
rc_immutable_string::c_str() const noexcept {
  return data_->c_str();
}

rc_immutable_string::size_type rc_immutable_string::size() const noexcept {
  return data_->size();
}

rc_immutable_string::const_iterator
rc_immutable_string::begin() const noexcept {
  return data_->begin();
}

rc_immutable_string::const_iterator
rc_immutable_string::cbegin() const noexcept {
  return data_->cbegin();
}

rc_immutable_string::const_iterator rc_immutable_string::end() const noexcept {
  return data_->end();
}

rc_immutable_string::const_iterator rc_immutable_string::cend() const noexcept {
  return data_->cend();
}

rc_immutable_string::const_reverse_iterator
rc_immutable_string::rbegin() const noexcept {
  return data_->rbegin();
}

rc_immutable_string::const_reverse_iterator
rc_immutable_string::rend() const noexcept {
  return data_->rend();
}

rc_immutable_string::const_reverse_iterator
rc_immutable_string::crbegin() const noexcept {
  return data_->crbegin();
}

rc_immutable_string::const_reverse_iterator
rc_immutable_string::crend() const noexcept {
  return data_->crend();
}

} // namespace wrap
