#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>
#include <iosfwd>
#include <semaphore>
#include <shared_mutex>
#include <string>
#include <unordered_set>

#include <fmt/format.h>

namespace plugin {

using action_id = uint64_t;

class error : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

class action_error : public error {
public:
  using error::error;
};

class action_does_not_exist : public action_error {
public:
  explicit action_does_not_exist(action_id x) : action_error(message(x)) {}

private:
  std::string message(action_id x) const {
    return fmt::format("Action {} does not exist", x);
  }
};

class action_already_exists : public action_error {
public:
  explicit action_already_exists(action_id x) : action_error(message(x)) {}

private:
  std::string message(action_id x) const {
    return fmt::format("Action {} already exists", x);
  }
};

class execution_exception : public action_error {
public:
  explicit execution_exception(action_id x) : action_error(message(x)) {}

private:
  std::string message(action_id x) const {
    return fmt::format("Execution error of action {}", x);
  }
};

class invalid_path : public std::system_error {
public:
  using system_error::system_error;
};

enum class configuration_status : uint32_t {
  finish,
  cancel,
};

struct plugin_version_t {
  uint16_t major;
  uint16_t minor;
  uint16_t patch;
  std::string pre_release;
  std::string build;

  plugin_version_t();
};

struct plugin_descriptor_t {
  std::string name;
  std::string description;

  plugin_descriptor_t();
};

struct plugin_attributes_t {
  std::function<void(action_id)> on_action_modified = [](action_id) {};
  std::function<void(action_id)> on_action_added = [](action_id) {};
  std::function<void(action_id)> on_action_removed = [](action_id) {};
  std::filesystem::path persistence_path;
};

struct action_descriptor_t {
public:
  action_descriptor_t(std::string name, std::string desc);

  const std::string &name() const noexcept;
  const std::string &description() const noexcept;

private:
  std::string name_;
  std::string desc_;
};

class action {
public:
  struct hash {
    using is_transparent = void;
    size_t operator()(const action &x) const;
    size_t operator()(action_id x) const;
  };

  struct equal {
    using is_transparent = void;
    bool operator()(const action &lhs, const action &rhs) const;
    bool operator()(const action &lhs, action_id rhs) const;
    bool operator()(action_id lhs, const action &rhs) const;
    bool operator()(action_id lhs, action_id rhs) const;
  };

  explicit action(action_descriptor_t);
  action(const action &);
  action(action &&);

  action &operator=(const action &);
  action &operator=(action &&);

  friend void swap(action &, action &);

  action_id id() const;
  action_descriptor_t descriptor() const;

  void descriptor(action_descriptor_t);

  void execute(std::ostream &os, int32_t val);

private:
  using mutex_t = std::shared_timed_mutex;
  using readlock_t = std::shared_lock<mutex_t>;
  using writelock_t = std::unique_lock<mutex_t>;

  action(const action &, readlock_t);
  action(action &&, writelock_t);

  action_id id_;
  action_descriptor_t desc_;
  mutable mutex_t mut_;
};

struct my_plugin {
  using config_callback_t =
      std::function<void(std::exception_ptr, configuration_status)>;

  // disable assignment and copying
  my_plugin(const my_plugin &) = delete;
  my_plugin &operator=(const my_plugin &) = delete;

  void execute(action_id, int32_t) const;

  action retrieve(action_id) const;
  std::vector<action_id> snapshot() const;
  bool configure(config_callback_t);

  static my_plugin &create(const plugin_attributes_t &attr) {
    static std::mutex mux;
    return create(attr, std::unique_lock{mux});
  }

  void addref() noexcept { count_.fetch_add(1); }

  void release() noexcept {
    count_.fetch_sub(1);
    if (0 == count_)
      delete this;
  }

private:
  using mutex_t = std::shared_timed_mutex;
  using readlock_t = std::shared_lock<mutex_t>;
  using writelock_t = std::unique_lock<mutex_t>;

  static my_plugin &create(const plugin_attributes_t &attr,
                           std::unique_lock<std::mutex>) {
    static my_plugin *ptr = nullptr;
    if (!ptr) {
      ptr = new my_plugin(attr);
    } else {
      ptr->addref();
    }
    return *ptr;
  }

  my_plugin(const plugin_attributes_t &);
  ~my_plugin();

  void insert(action);
  void modify(action);
  void remove(action_id);

  void configuration_procedure(std::stop_token);

  std::atomic<int64_t> count_;
  plugin_attributes_t attr_;
  std::unordered_set<action, action::hash, action::equal> actions_;
  mutable mutex_t mtx_;

  mutable std::binary_semaphore sem_to_cfg_;
  mutable std::binary_semaphore sem_from_cfg_;
  config_callback_t cfgcallback_;

  std::jthread modifier_;
  std::jthread configurator_;
};

void swap(action &, action &);

} // namespace plugin
