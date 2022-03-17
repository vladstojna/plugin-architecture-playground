#include "plugin_impl.hpp"

#include <atomic>
#include <iostream>

namespace {

plugin::action_id next_id() {
  static std::atomic<plugin::action_id> id = 0;
  return ++id;
}

} // namespace

namespace plugin {

plugin_version_t::plugin_version_t()
    : major(0), minor(0), patch(1), pre_release("alpha"), build("12345") {}

plugin_descriptor_t::plugin_descriptor_t()
    : name("my_test_plugin"),
      description(
          "Exists for nothing other than to implement the plugin interface") {}

action::action(action_descriptor_t desc)
    : id_(next_id()), desc_(std::move(desc)) {}

action::action(const action &x) : action(x, readlock_t{x.mut_}) {}

action::action(action &&x) : action(std::move(x), writelock_t{x.mut_}) {}

action::action(const action &x, readlock_t lk) : id_(x.id_), desc_(x.desc_) {}

action::action(action &&x, writelock_t lk)
    : id_(std::move(x.id_)), desc_(std::move(x.desc_)) {}

action &action::operator=(const action &x) {
  if (this != &x) {
    writelock_t lhs_lk{mut_, std::defer_lock};
    readlock_t rhs_lk{x.mut_, std::defer_lock};
    std::lock(lhs_lk, rhs_lk);
    id_ = x.id_;
    desc_ = x.desc_;
  }
  return *this;
}

action &action::operator=(action &&x) {
  if (this != &x) {
    writelock_t lhs_lk{mut_, std::defer_lock};
    writelock_t rhs_lk{x.mut_, std::defer_lock};
    std::lock(lhs_lk, rhs_lk);
    id_ = std::move(x.id_);
    desc_ = std::move(x.desc_);
  }
  return *this;
}

action_id action::id() const {
  readlock_t lk(mut_);
  return id_;
}

action_descriptor_t action::descriptor() const {
  readlock_t lk(mut_);
  return desc_;
}

void action::descriptor(action_descriptor_t x) {
  writelock_t lk(mut_);
  desc_ = std::move(x);
}

void action::execute(std::ostream &os, int32_t val) {
  readlock_t lk(mut_);
  if (val > 0)
    os << "positive value ";
  else if (val < 0)
    throw execution_exception(id());
  else
    os << "zero ";
  os << val << "\n";
}

action_descriptor_t::action_descriptor_t(std::string name, std::string desc)
    : name_(std::move(name)), desc_(std::move(desc)) {}

const std::string &action_descriptor_t::name() const noexcept { return name_; }

const std::string &action_descriptor_t::description() const noexcept {
  return desc_;
}

size_t action::hash::operator()(const action &x) const {
  return std::hash<decltype(x.id())>{}(x.id());
}

size_t action::hash::operator()(action_id x) const {
  return std::hash<decltype(x)>{}(x);
}

bool action::equal::operator()(const action &lhs, const action &rhs) const {
  return lhs.id() == rhs.id();
}

bool action::equal::operator()(const action &lhs, action_id rhs) const {
  return lhs.id() == rhs;
}

bool action::equal::operator()(action_id lhs, const action &rhs) const {
  return lhs == rhs.id();
}

bool action::equal::operator()(action_id lhs, action_id rhs) const {
  return lhs == rhs;
}

void swap(action &x, action &y) {
  if (&x != &y) {
    action::writelock_t lhs_lk{x.mut_, std::defer_lock};
    action::writelock_t rhs_lk{y.mut_, std::defer_lock};
    std::lock(lhs_lk, rhs_lk);
    using std::swap;
    swap(x.id_, y.id_);
    swap(x.desc_, y.desc_);
  }
}

my_plugin::my_plugin(const plugin_attributes_t &attr)
    : count_(1), attr_(attr), sem_to_cfg_(0), sem_from_cfg_(1),
      modifier_([this]() {
        constexpr size_t count = 10;
        for (size_t i = 0; i < count; i++) {
          insert(action{action_descriptor_t{
              std::string("action").append(std::to_string(i)),
              "prints value"}});
        }
        for (size_t i = 0; i < count / 2; i++) {
          remove(i + 1);
        }
      }),
      configurator_(
          [this](std::stop_token stoken) { configuration_procedure(stoken); }) {
  std::error_code ec;
  if (!std::filesystem::is_directory(attr_.persistence_path, ec))
    throw invalid_path(std::move(ec));
  auto status = std::filesystem::status(attr_.persistence_path, ec);
  if (ec)
    throw invalid_path(std::move(ec));
  using std::filesystem::perms;
  if (perms::none == (status.permissions() & perms::owner_write))
    throw invalid_path(std::make_error_code(std::errc::permission_denied));
}

my_plugin::~my_plugin() {
  configurator_.request_stop();
  sem_to_cfg_.release();
}

void my_plugin::insert(action x) {
  writelock_t lk(mtx_);
  auto [it, inserted] = actions_.insert(std::move(x));
  if (!inserted)
    throw action_already_exists(x.id());
  attr_.on_action_added(it->id());
}

void my_plugin::modify(action x) {
  writelock_t lk(mtx_);
  auto it = actions_.find(x.id());
  if (it == actions_.end())
    throw action_does_not_exist(x.id());
  // modifiable values do not affect the hash computation
  const_cast<action &>(*it) = std::move(x);
  attr_.on_action_modified(it->id());
}

void my_plugin::remove(action_id id) {
  writelock_t lk(mtx_);
  auto it = actions_.find(id);
  if (it == actions_.end())
    throw action_does_not_exist(id);
  actions_.erase(it);
  attr_.on_action_removed(id);
}

void my_plugin::configuration_procedure(std::stop_token stoken) {
  while (true) {
    sem_to_cfg_.acquire();
    if (stoken.stop_requested())
      break;
    try {
      std::this_thread::sleep_for(std::chrono::milliseconds(2000));
      cfgcallback_(nullptr, configuration_status::finish);
    } catch (...) {
      cfgcallback_(std::current_exception(), {});
    }
    sem_from_cfg_.release();
  }
}

bool my_plugin::configure(config_callback_t cb) {
  bool res = sem_from_cfg_.try_acquire();
  if (res) {
    cfgcallback_ = std::move(cb);
    sem_to_cfg_.release();
  }
  return res;
}

void my_plugin::execute(action_id id, int32_t val) const {
  readlock_t lk(mtx_);
  auto it = actions_.find(id);
  if (it == actions_.end())
    throw action_does_not_exist(id);
  const_cast<action &>(*it).execute(std::cout, val);
}

action my_plugin::retrieve(action_id id) const {
  readlock_t lk(mtx_);
  auto it = actions_.find(id);
  if (it == actions_.end())
    throw action_does_not_exist(id);
  return *it;
}

std::vector<action_id> my_plugin::snapshot() const {
  readlock_t lk(mtx_);
  std::vector<action_id> ids;
  ids.reserve(actions_.size());
  for (const auto &a : actions_)
    ids.push_back(a.id());
  return ids;
}

} // namespace plugin
