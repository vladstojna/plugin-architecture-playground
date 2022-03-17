#include "..\include\wrap\plugin_configurator.hpp"
#include <wrap/error.hpp>
#include <wrap/error_descriptor.hpp>
#include <wrap/plugin_configurator.hpp>

#include <module_load/module.hpp>

#include "status_utils.hpp"

#include <cassert>
#include <iostream>
#include <semaphore>

#pragma warning(error : 4062)

namespace {

std::pair<std::error_code, wrap::configure_status>
to_status_pair(PASMP_status_t s, PASMP_config_status_t x) noexcept {
  auto ec = wrap::make_error_code(s);
  if (ec)
    return {std::move(ec), {}};
  switch (x) {
  case PASMP_CONFIG_SUCCESS:
    return {std::move(ec), wrap::configure_status::success};
  case PASMP_CONFIG_CANCEL:
    return {std::move(ec), wrap::configure_status::cancel};
  }
  return {wrap::make_error_code(wrap::logic_errc::invalid_status), {}};
}

template <typename T> struct configurator_impl_base {
  wrap::plugin plugin_;
  wrap::configure_result result_;
  std::atomic<bool> being_configured_;
  std::binary_semaphore resultsem_;

  explicit configurator_impl_base(const wrap::plugin &) noexcept;

  std::error_code prime_configure(wrap::configure_mode,
                                  wrap::error_descriptor &) noexcept;

  std::error_code configure(wrap::configure_mode,
                            wrap::error_descriptor &) noexcept;

  wrap::configure_result await() noexcept;

protected:
  ~configurator_impl_base();
};

template <typename T>
configurator_impl_base<T>::configurator_impl_base(
    const wrap::plugin &p) noexcept
    : plugin_(p), being_configured_(false), resultsem_(0) {}

template <typename T> configurator_impl_base<T>::~configurator_impl_base() {
  await();
}
template <typename T>
wrap::configure_result configurator_impl_base<T>::await() noexcept {
  if (bool exp = true; !being_configured_.compare_exchange_strong(exp, false))
    return {wrap::make_error_code(wrap::plugin_errc::unavailable), {}};
  resultsem_.acquire();
  return result_;
}

template <typename T>
std::error_code configurator_impl_base<T>::prime_configure(
    wrap::configure_mode mode, wrap::error_descriptor &ed) noexcept {
  using namespace wrap;
  auto status = PASMP_SUCCESS;
  ed.clear();
  switch (mode) {
  case configure_mode::cli:
    status = plugin_.get_module().funcs().plugin_configure_cli(
        plugin_.get(), &T::wrap_callback, static_cast<T *>(this), &ed);
    return make_error_code(status);
  case configure_mode::gui:
    status = plugin_.get_module().funcs().plugin_configure_gui(
        plugin_.get(), &T::wrap_callback, static_cast<T *>(this), &ed);
    return make_error_code(status);
  };
  assert(false);
  return make_error_code(logic_errc::invalid_argument);
}

template <typename T>
std::error_code
configurator_impl_base<T>::configure(wrap::configure_mode mode,
                                     wrap::error_descriptor &ed) noexcept {
  auto ec = prime_configure(mode, ed);
  if (!ec)
    being_configured_ = true;
  return ec;
}

} // namespace

namespace wrap {

struct plugin_configurator::impl : configurator_impl_base<impl> {
  using configurator_impl_base::configurator_impl_base;

  static void PASMP_CALLBACK wrap_callback(PASMP_status_t,
                                           PASMP_config_status_t, void *);

  // shadow base class configure
  configure_result configure(wrap::configure_mode,
                             wrap::error_descriptor &) noexcept;
};

struct plugin_configurator_async::impl : configurator_impl_base<impl> {
  callback_t callback_;
  on_error_t on_error_;

  impl(const plugin &, callback_t &&, on_error_t &&) noexcept;

  static void PASMP_CALLBACK wrap_callback(PASMP_status_t,
                                           PASMP_config_status_t, void *);
};

void plugin_configurator::impl::wrap_callback(PASMP_status_t s,
                                              PASMP_config_status_t cs,
                                              void *data) {
  try {
    auto &imp = *static_cast<impl *>(data);
    imp.result_ = to_status_pair(s, cs);
    imp.resultsem_.release();
  } catch (...) {
    handle_callback_exception();
  }
}

configure_result
plugin_configurator::impl::configure(wrap::configure_mode mode,
                                     wrap::error_descriptor &ed) noexcept {
  auto ec = configurator_impl_base::configure(mode, ed);
  if (ec)
    return {ec, {}};
  return await();
}

plugin_configurator_async::impl::impl(const plugin &p, callback_t &&c,
                                      on_error_t &&e) noexcept
    : configurator_impl_base(p), callback_(std::move(c)),
      on_error_(std::move(e)) {}

void plugin_configurator_async::impl::wrap_callback(PASMP_status_t s,
                                                    PASMP_config_status_t cs,
                                                    void *data) {
  auto &imp = *static_cast<impl *>(data);
  try {
    auto [ec, stat] = to_status_pair(s, cs);
    assert(imp.callback_);
    imp.callback_(ec, stat);
    imp.result_ = {ec, stat};
  } catch (...) {
    imp.result_ = {make_error_code(logic_errc::callback_exception), {}};
    if (imp.on_error_) {
      try {
        imp.on_error_(std::current_exception());
      } catch (...) {
        handle_callback_exception();
      }
    } else {
      handle_callback_exception();
    }
    imp.resultsem_.release();
  }
}

plugin_configurator::plugin_configurator(const plugin &p)
    : impl_(std::make_unique<impl>(p)) {}

plugin_configurator::plugin_configurator(plugin_configurator &&) noexcept =
    default;

plugin_configurator &
plugin_configurator::operator=(plugin_configurator &&) noexcept = default;

plugin_configurator::~plugin_configurator() = default;

const plugin &plugin_configurator::get_plugin() const noexcept {
  return impl_->plugin_;
}

configure_status plugin_configurator::configure(configure_mode mode,
                                                error_descriptor &ed) {
  auto [ec, status] = configure(mode, ed, nothrow);
  if (ec)
    error_code_as_exception(ec, ed);
  return status;
}

configure_status plugin_configurator::operator()(configure_mode mode,
                                                 error_descriptor &ed) {
  return configure(mode, ed);
}

configure_result plugin_configurator::configure(configure_mode mode,
                                                error_descriptor &ed,
                                                nothrow_t) noexcept {
  return impl_->configure(mode, ed);
}

configure_result plugin_configurator::operator()(configure_mode mode,
                                                 error_descriptor &ed,
                                                 nothrow_t tag) noexcept {
  return configure(mode, ed, tag);
}

plugin_configurator_async::plugin_configurator_async(const plugin &p,
                                                     callback_t c, on_error_t e)
    : impl_(std::make_unique<impl>(p, std::move(c), std::move(e))) {}

plugin_configurator_async::plugin_configurator_async(
    plugin_configurator_async &&) noexcept = default;

plugin_configurator_async &plugin_configurator_async::operator=(
    plugin_configurator_async &&) noexcept = default;

plugin_configurator_async::~plugin_configurator_async() = default;

const plugin &plugin_configurator_async::get_plugin() const noexcept {
  return impl_->plugin_;
}

void plugin_configurator_async::configure(configure_mode d,
                                          error_descriptor &ed) {
  if (std::error_code ec; !configure(d, ec, ed))
    error_code_as_exception(ec, ed);
}

void plugin_configurator_async::operator()(configure_mode d,
                                           error_descriptor &ed) {
  configure(d, ed);
}

bool plugin_configurator_async::configure(configure_mode d, std::error_code &ec,
                                          error_descriptor &ed) noexcept {
  return !bool(ec = impl_->configure(d, ed));
}

bool plugin_configurator_async::operator()(configure_mode d,
                                           std::error_code &ec,
                                           error_descriptor &ed) noexcept {
  return configure(d, ec, ed);
}

configure_result plugin_configurator_async::await() noexcept {
  return impl_->await();
}

} // namespace wrap

#pragma warning(disable : 4062)
