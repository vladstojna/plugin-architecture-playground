#pragma once

#include <wrap/nothrow.hpp>
#include <wrap/plugin.hpp>
#include <wrap/visibility.hpp>

#include <functional>
#include <system_error>

namespace wrap {

struct error_descriptor;

enum class configure_mode : uint32_t {
  cli,
  gui,
};

enum class configure_status : uint32_t {
  success,
  cancel,
};

using configure_result = std::pair<std::error_code, configure_status>;

class WRAPPER_DLL_PUBLIC plugin_configurator {
public:
  explicit plugin_configurator(const plugin &);

  plugin_configurator(plugin_configurator &&) noexcept;
  plugin_configurator &operator=(plugin_configurator &&) noexcept;

  ~plugin_configurator();

  const plugin &get_plugin() const noexcept;

  configure_status configure(configure_mode, error_descriptor &);
  configure_status operator()(configure_mode, error_descriptor &);

  configure_result configure(configure_mode, error_descriptor &,
                             nothrow_t) noexcept;
  configure_result operator()(configure_mode, error_descriptor &,
                              nothrow_t) noexcept;

private:
  struct impl;
  std::unique_ptr<impl> impl_;
};

class WRAPPER_DLL_PUBLIC plugin_configurator_async {
public:
  using callback_t = std::function<void(std::error_code, configure_status)>;
  using on_error_t = std::function<void(std::exception_ptr)>;

  plugin_configurator_async(const plugin &, callback_t, on_error_t = {});

  plugin_configurator_async(plugin_configurator_async &&) noexcept;
  plugin_configurator_async &operator=(plugin_configurator_async &&) noexcept;

  ~plugin_configurator_async();

  const plugin &get_plugin() const noexcept;

  void configure(configure_mode, error_descriptor &);
  void operator()(configure_mode, error_descriptor &);

  bool configure(configure_mode, std::error_code &,
                 error_descriptor &) noexcept;
  bool operator()(configure_mode, std::error_code &,
                  error_descriptor &) noexcept;

  configure_result await() noexcept;

private:
  struct impl;
  std::unique_ptr<impl> impl_;
};

} // namespace wrap
