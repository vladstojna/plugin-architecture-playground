#include <wrap/action.hpp>
#include <wrap/error.hpp>
#include <wrap/error_descriptor.hpp>
#include <wrap/plugin.hpp>
#include <wrap/plugin_attributes.hpp>
#include <wrap/plugin_object.hpp>

#include <module_load/module.hpp>
#include <plugin/plugin_interface.h>

#include "status_utils.hpp"

#include <cassert>

namespace {

class action_collection {
public:
  explicit action_collection(const wrap::plugin &p, wrap::error_descriptor &ed,
                             wrap::key<wrap::plugin> key)
      : col_(create_collection(ed)), plugin_(p), key_(key) {}

  ~action_collection() {
    if (auto status =
            plugin_.get_module().funcs().action_collection_destroy(col_))
      wrap::default_error_handler("Error destroying action collection",
                                  wrap::make_error_code(status));
  }

  action_collection(const action_collection &) = delete;
  action_collection &operator=(const action_collection &) = delete;

  wrap::action operator[](size_t idx) const {
    wrap::null_error_descriptor ed;
    return at(idx, ed);
  }

  wrap::action at(size_t idx, wrap::error_descriptor &ed) const {
    PASMP_action_t act = nullptr;
    ed.clear();
    if (auto status = plugin_.get_module().funcs().action_collection_at(
            get(), idx, &act, &ed))
      wrap::status_to_exception(status, ed);
    return wrap::action{plugin_, act, key_};
  }

  std::optional<wrap::action> at(size_t idx, std::error_code &ec,
                                 wrap::error_descriptor &ed) const {
    PASMP_action_t act = nullptr;
    ed.clear();
    if (auto status = plugin_.get_module().funcs().action_collection_at(
            get(), idx, &act, &ed)) {
      ec = wrap::make_error_code(status);
      return std::nullopt;
    }
    ed.clear();
    return wrap::action{plugin_, act, key_};
  }

  size_t size() const noexcept {
    return plugin_.get_module().funcs().action_collection_size(get());
  }

  PASMP_action_collection_t get() const noexcept { return col_; }

private:
  PASMP_action_collection_t
  create_collection(wrap::error_descriptor &ed) const {
    PASMP_action_collection_t col = nullptr;
    ed.clear();
    if (auto status =
            plugin_.get_module().funcs().action_collection_create(&col, &ed))
      wrap::status_to_exception(status, ed);
    return col;
  }

  PASMP_action_collection_t col_;
  wrap::plugin plugin_;
  wrap::key<wrap::plugin> key_;
};

class plugin_attributes_helper final : public ::wrap::plugin_object {
public:
  PASMP_plugin_attr_t get() const noexcept { return attr_; }

  plugin_attributes_helper(const modl::loaded_module &mod,
                           wrap::error_descriptor &ed)
      : plugin_object(mod), attr_(create_attr(ed)) {}

  ~plugin_attributes_helper() {
    if (attr_)
      if (auto status = get_module().funcs().plugin_attr_destroy(attr_))
        wrap::default_error_handler("Error destroying plugin attributes helper",
                                    wrap::make_error_code(status));
  }

  plugin_attributes_helper(plugin_attributes_helper &&x) noexcept
      : plugin_object(std::move(x)), attr_(std::exchange(x.attr_, nullptr)) {}

  plugin_attributes_helper &operator=(plugin_attributes_helper &&x) noexcept {
    if (this != &x) {
      plugin_object::operator=(std::move(x));
      attr_ = std::exchange(x.attr_, nullptr);
    }
    return *this;
  }

  plugin_attributes_helper(const plugin_attributes_helper &) = delete;
  plugin_attributes_helper &
  operator=(const plugin_attributes_helper &) = delete;

private:
  PASMP_plugin_attr_t create_attr(wrap::error_descriptor &ed) {
    PASMP_plugin_attr_t attr = nullptr;
    ed.clear();
    if (auto status = get_module().funcs().plugin_attr_create(&attr, &ed))
      wrap::status_to_exception(status, ed);
    return attr;
  }

private:
  PASMP_plugin_attr_t attr_;
};

} // namespace

namespace wrap {

struct plugin::impl final : plugin_object,
                            std::enable_shared_from_this<plugin::impl> {
  plugin_attributes attr;
  PASMP_plugin_t handle;

  static std::shared_ptr<impl> create(const modl::loaded_module &mod,
                                      plugin_attributes &&attr,
                                      error_descriptor &ed) {
    std::shared_ptr<impl> ptr(new impl{mod, std::move(attr), ed});
    ptr->init(ed);
    return ptr;
  }

  ~impl() {
    if (handle)
      if (auto status = get_module().funcs().plugin_release(handle))
        default_error_handler("Error destroying plugin",
                              make_error_code(status));
  }

private:
  impl(const modl::loaded_module &mod, plugin_attributes &&attr,
       error_descriptor &ed)
      : plugin_object(mod), attr(std::move(attr)), handle(nullptr) {}

  void init(error_descriptor &ed) { handle = create_plugin(init_attr(ed), ed); }

  plugin_attributes_helper init_attr(error_descriptor &ed) {
    plugin_attributes_helper attr_h(get_module(), ed);
    {
      ed.clear();
      auto status = get_module().funcs().plugin_attr_on_action_add(
          attr_h.get(), &wrap_callback<action_event::add>, this, &ed);
      if (status)
        wrap::status_to_exception(status, ed);
    }
    {
      ed.clear();
      auto status = get_module().funcs().plugin_attr_on_action_mod(
          attr_h.get(), &wrap_callback<action_event::modify>, this, &ed);
      if (status)
        wrap::status_to_exception(status, ed);
    }
    {
      ed.clear();
      auto status = get_module().funcs().plugin_attr_on_action_rm(
          attr_h.get(), &wrap_callback<action_event::remove>, this, &ed);
      if (status)
        wrap::status_to_exception(status, ed);
    }
    {
      ed.clear();
      std::string path_str = attr.persistence_path().string();
      auto status = get_module().funcs().plugin_attr_persistence_path(
          attr_h.get(),
          PASMP_string_view_t{.data = path_str.c_str(),
                              .size = path_str.size()},
          &ed);
      if (status)
        wrap::status_to_exception(status, ed);
    }
    return attr_h;
  }

  PASMP_plugin_t create_plugin(plugin_attributes_helper attr,
                               error_descriptor &ed) const {
    PASMP_plugin_t plug = nullptr;
    ed.clear();
    if (auto status =
            get_module().funcs().plugin_create(&plug, attr.get(), &ed))
      wrap::status_to_exception(status, ed);
    return plug;
  }

  template <action_event Event>
  static void PASMP_CALLBACK wrap_callback(PASMP_action_t handle, void *data) {
    static_assert(Event >= action_event::add && Event <= action_event::remove,
                  "Incorrect event in wrap_callback");
    const auto &imp = *static_cast<const impl *>(data);
    key<plugin> k;
    try {
      imp.attr.callback(k)(Event, action{plugin(imp), handle, k});
    } catch (...) {
      const auto &on_error = imp.attr.on_error(k);
      if (on_error) {
        try {
          on_error(std::current_exception());
        } catch (...) {
          handle_callback_exception();
        }
      } else {
        handle_callback_exception();
      }
    }
  }
};

plugin::plugin(const modl::loaded_module &mod, plugin_attributes attr,
               error_descriptor &ed)
    : impl_(impl::create(mod, std::move(attr), ed)) {}

plugin::plugin(const impl &x) : impl_(x.shared_from_this()) {}

std::vector<action> plugin::actions(error_descriptor &ed) const {
  action_collection col(*this, ed, key<plugin>{});
  ed.clear();
  if (auto status = get_module().funcs().plugin_actions(get(), col.get(), &ed))
    wrap::status_to_exception(status, ed);
  std::vector<action> actions;
  actions.reserve(col.size());
  for (size_t ix = 0; ix < col.size(); ix++)
    actions.emplace_back(col.at(ix, ed));
  return actions;
}

std::vector<action> plugin::actions(std::error_code &ec,
                                    error_descriptor &ed) const {
  action_collection col(*this, ed, key<plugin>{});
  std::vector<action> actions;
  ed.clear();
  if (auto status =
          get_module().funcs().plugin_actions(get(), col.get(), &ed)) {
    ec = make_error_code(status);
    return actions;
  }
  actions.reserve(col.size());
  for (size_t ix = 0; ix < col.size(); ix++) {
    auto opt_handle = col.at(ix, ec, ed);
    if (ec) {
      actions.clear();
      return actions;
    }
    assert(opt_handle);
    actions.emplace_back(*std::move(opt_handle));
  }
  ec.clear();
  return actions;
}

PASMP_plugin_t plugin::get() const noexcept { return impl_->handle; }

const modl::loaded_module &plugin::get_module() const noexcept {
  return impl_->get_module();
}

const plugin_attributes &plugin::attributes() const noexcept {
  return impl_->attr;
}

bool operator==(const plugin &lhs, const plugin &rhs) {
  return lhs.get_module() == rhs.get_module() && lhs.get() == rhs.get();
}

} // namespace wrap
