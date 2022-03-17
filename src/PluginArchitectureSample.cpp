#include <fmt/format.h>
#include <midi/midi.hpp>
#include <module_load/exception.hpp>
#include <module_load/module.hpp>
#include <spdlog/spdlog.h>
#include <wrap/wrap.hpp>

#include "blocking_queue.hpp"
#include "misc.hpp"

#include <cassert>
#include <iostream>
#include <random>
#include <string_view>
#include <unordered_set>

namespace frontend {

struct message_t {
  wrap::action_event event;
  wrap::action act;
};

using queue_t = q::blocking_queue<message_t, 256>;

void on_action_event(queue_t &queue, wrap::action_event e, wrap::action &&h) {
  queue.emplace(e, std::move(h));
}

void on_midi_message(const midi::midiin_descriptor_t &descr,
                     midi::message_type m, midi::message_data d) {
  switch (m) {
  case midi::message_type::open:
    spdlog::info("{}:{} opened", descr.port, descr.name);
    return;
  case midi::message_type::close:
    spdlog::info("{}:{} closed", descr.port, descr.name);
    return;
  case midi::message_type::data:
    spdlog::info("{}:{}: {}:{},{:#04x},{:#04x}", descr.port, descr.name,
                 d.timestamp.time_since_epoch().count(), d.status.descr(),
                 d.data[0], d.data[1]);
    return;
  case midi::message_type::sys_excl_done:
  case midi::message_type::other:
    spdlog::info("{}:{}: Other MIDI message", descr.port, descr.name);
    return;
  case midi::message_type::error:
  case midi::message_type::sys_excl_error:
    spdlog::error("{}:{}: MIDI error message", descr.port, descr.name);
    break;
  }
  throw midi::midi_error(midi::errc::bad_message, "Error on message");
}

} // namespace frontend

static std::ostream &operator<<(std::ostream &os, modl::library_version v) {
  os << v.major << "." << v.minor;
  return os;
}

int main() {
  try {
    {
      midi::set_default_error_handler(
          [](const char *msg, std::error_code ec) noexcept {
            try {
              spdlog::error("({}:{}) {} | {}", ec.category().name(), ec.value(),
                            msg, ec.message());
            } catch (const std::exception &e) {
              spdlog::error(e.what());
            } catch (...) {
              spdlog::error("Unknown error in default error handler");
            }
          });

      midi::midiin_attributes mattr;
      mattr.on_message(
          [](const midi::midiin_descriptor_t &descr, midi::message_type m,
             midi::message_data d) { frontend::on_midi_message(descr, m, d); });
      mattr.on_disconnect([](const midi::midiin_descriptor_t &d) {
        spdlog::error("{}:{} disconnected", d.port, d.name);
      });
      mattr.polling_interval(midi::midiin_attributes::interval_t{1000});

      midi::midiin midiin(midi::midiin_descriptor_t(0), std::move(mattr));
      std::cin.get();
    }

    std::random_device rnd_dev;
    std::mt19937_64 engine{rnd_dev()};
    std::uniform_int_distribution<int32_t> dist{-100, 100};

    const auto &path = frontend::persistence_path_prefix();
    auto plugin_path = std::filesystem::path("plugin") / "MyPlugin2.dll";
    auto persistence_path = path / "PluginArchitectureSample" /
                            plugin_path.parent_path() / plugin_path.stem();

    std::filesystem::create_directories(persistence_path);

    modl::loaded_module mod{plugin_path};

    wrap::set_default_error_handler(
        [](const char *msg, std::error_code ec) noexcept {
          try {
            spdlog::error("({}:{}) {} | {}", ec.category().name(), ec.value(),
                          msg, ec.message());
          } catch (const std::exception &e) {
            spdlog::error(e.what());
          } catch (...) {
            spdlog::error("Unknown error in unusual error handler");
          }
        });

    frontend::queue_t message_queue;
    wrap::static_error_descriptor<128> ed;
    wrap::plugin_version version(mod, ed);
    wrap::plugin_descriptor descriptor(mod, ed);
    wrap::plugin_attributes attr(
        persistence_path,
        [&message_queue](wrap::action_event e, wrap::action x) {
          frontend::on_action_event(message_queue, e, std::move(x));
        });
    wrap::plugin plugin(mod, std::move(attr), ed);
    wrap::plugin plugin2 = plugin;
    wrap::plugin_configurator_async pc(plugin2, [](auto, auto) {
      throw std::runtime_error("hello from callback");
    });

    std::cout << "Module implements library version: " << mod.version() << "\n";
    std::cout << version << "\n";
    std::cout << descriptor << "\n";
    std::cout << plugin2.attributes().persistence_path().string() << "\n";

    while (!message_queue.empty()) {
      auto msg = message_queue.pop();
      switch (msg.event) {
      case wrap::action_event::add: {
        std::cout << "Action add\n";
        try {
          std::cout << wrap::action_descriptor{msg.act, ed} << std::endl;
        } catch (const wrap::any_error &e) {
          spdlog::error("({}:{}) Error getting descriptor: {} - {}",
                        e.code().category().name(), e.code().value(),
                        e.code().message(), e.what());
        }
        wrap::action_executor exec(
            msg.act, [](std::error_code ec, std::string_view err_msg) {
              throw std::runtime_error("hello from executor callback");
            });
        for (size_t i = 0; i < 10; i++) {
          if (std::error_code code;
              !exec.submit(wrap::payload{dist(engine)}, code, ed)) {
            if (code == wrap::plugin_errc::action_not_found) {
              spdlog::warn(
                  "Action '{}' does not exist, will receive removed event",
                  to_string(msg.act));
            } else if (code) {
              spdlog::error("({}:{}) Error getting descriptor: {}",
                            code.category().name(), code.value(),
                            code.message());
            }
          }
        }
        exec.await();
      } break;
      case wrap::action_event::remove:
        std::cout << "Action remove\n";
        break;
      case wrap::action_event::modify:
        std::cout << "Action modify\n";
        break;
      }
    }

    if (std::error_code ec; !pc(wrap::configure_mode::cli, ec, ed)) {
      spdlog::error("({}:{}) Configuration request failed: {}",
                    ec.category().name(), ec.value(), ec.message());
    } else {
      if (auto [code, st] = pc.await(); code) {
        spdlog::error("({}:{}) Configuration failed: {}",
                      code.category().name(), code.value(), code.message());
      } else {
        using wrap::configure_status;
        switch (st) {
        case configure_status::cancel:
          std::cout << "configuration succeeded but cancelled\n";
          break;
        case configure_status::success:
          std::cout << "configuration finished\n";
          break;
        }
      }
    }
  } catch (const modl::function_load_error &e) {
    std::cerr << e.code().message() << ": " << e.name() << "\n";
    return 1;
  } catch (const wrap::any_error &e) {
    std::cerr << e.code().message() << ": " << e.what() << "\n";
    return 1;
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Unknown exception caught" << std::endl;
    return 1;
  }
}
