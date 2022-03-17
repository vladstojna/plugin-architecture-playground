#include <wrap/error_descriptor.hpp>

#include <cassert>

namespace wrap {

error_descriptor::error_descriptor() noexcept : descr_{} {
  assert(!descr_.what && !descr_.size);
}

void error_descriptor::clear() noexcept { reset_size(); }

bool error_descriptor::empty() const noexcept { return available() == size(); }

PASMP_error_descriptor_t &error_descriptor::raw() &noexcept {
  descr_.what = buffer();
  return descr_;
}

std::string_view error_descriptor::view() const noexcept {
  return {buffer(), size() - available()};
}

void error_descriptor::reset_size() noexcept { descr_.size = size(); }

size_t error_descriptor::available() const noexcept { return descr_.size; }

const char *null_error_descriptor::buffer() const noexcept { return nullptr; }

char *null_error_descriptor::buffer() noexcept { return nullptr; }

size_t null_error_descriptor::size() const noexcept { return 0; }

PASMP_error_descriptor_t *null_error_descriptor::operator&() &noexcept {
  return nullptr;
}

dynamic_error_descriptor::dynamic_error_descriptor(size_t sz) {
  data_.resize(sz);
  clear();
}

const char *dynamic_error_descriptor::buffer() const noexcept {
  return data_.data();
}

char *dynamic_error_descriptor::buffer() noexcept { return data_.data(); }

size_t dynamic_error_descriptor::size() const noexcept { return data_.size(); }

} // namespace wrap
