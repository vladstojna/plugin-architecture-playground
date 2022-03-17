#include <plugin/plugin_interface.h>

#include "plugin_impl.hpp"

#include <cassert>
#include <charconv>
#include <iostream>
#include <string_view>

namespace {

void fill_error_descriptor(PASMP_error_descriptor_t *err,
                           std::string_view content) noexcept {
  if (!err || !err->size || !content.size())
    return;
  auto min = std::min(err->size, content.size());
  assert(min);
  std::strncpy(err->what, content.data(), min);
  if (min == err->size)
    err->what[min - 1] = 0;
  else
    err->what[min] = 0;
  err->size -= min;
}

PASMP_status_t unknown_error(PASMP_error_descriptor_t *err) noexcept {
  return PASMP_UNKNOWN;
}

template <typename> struct handle_traits {};

template <> struct handle_traits<PASMP_plugin_t> {
  static constexpr auto generic_error = PASMP_ERROR_PLUGIN;
};

template <>
struct handle_traits<PASMP_plugin_attr_t> : handle_traits<PASMP_plugin_t> {};

template <> struct handle_traits<PASMP_action_t> {
  static constexpr auto generic_error = PASMP_ERROR_ACTION_OTHER;
};

PASMP_status_t serialization_error(PASMP_error_descriptor_t *err,
                                   std::error_code ec) noexcept {
  try {
    auto msg = ec.message();
    fill_error_descriptor(err, msg);
  } catch (...) {
    fill_error_descriptor(err, "Error during de/serialization error");
  }
  return PASMP_ERROR_ACTION_SERIAL;
}

PASMP_status_t path_error(PASMP_error_descriptor_t *err,
                          const plugin::invalid_path &e) noexcept {
  fill_error_descriptor(err, e.what());
  return PASMP_ERROR_PERSISTENCE_PATH;
}

PASMP_status_t alloc_error(PASMP_error_descriptor_t *err,
                           std::string_view msg) noexcept {
  fill_error_descriptor(err, msg);
  return PASMP_ERROR_ALLOC;
}

template <typename T>
PASMP_status_t generic_error(PASMP_error_descriptor_t *err,
                             const std::exception &e) noexcept {
  fill_error_descriptor(err, e.what());
  return handle_traits<T>::generic_error;
}

PASMP_status_t system_error(PASMP_error_descriptor_t *err,
                            const std::system_error &e) noexcept {
  fill_error_descriptor(err, e.what());
  return PASMP_ERROR_SYSTEM;
}

PASMP_status_t fatal_error(PASMP_error_descriptor_t *err,
                           const std::logic_error &e) noexcept {
  fill_error_descriptor(err, e.what());
  return PASMP_ERROR_FATAL;
}

PASMP_status_t invalid_argument(PASMP_error_descriptor_t *err,
                                std::string_view msg) noexcept {
  fill_error_descriptor(err, msg);
  return PASMP_INVALID_ARGUMENT;
}

plugin::action_id remove_msb(plugin::action_id x) {
  return x & (std::numeric_limits<decltype(x)>::max() >> 1);
}

plugin::action_id to_id(PASMP_action_t x) {
  return remove_msb(std::bit_cast<plugin::action_id>(x));
}

} // namespace

#pragma region action_collection_impl

struct PASMP_action_collection_st {
  std::vector<PASMP_action_t> actions;
};

PASMP_status_t
PASMP_action_collection_create(PASMP_action_collection_t *out,
                               PASMP_error_descriptor_t *err_out) {
  if (!out)
    return PASMP_INVALID_ARGUMENT;
  try {
    *out = new PASMP_action_collection_st();
  } catch (...) {
    return alloc_error(err_out, "Error allocating space for action collection");
  }
  return PASMP_SUCCESS;
}

PASMP_status_t PASMP_action_collection_destroy(PASMP_action_collection_t col) {
  if (col)
    delete col;
  return PASMP_SUCCESS;
}

uint64_t PASMP_action_collection_size(PASMP_action_collection_t col) {
  return col->actions.size();
}

PASMP_status_t PASMP_action_collection_at(PASMP_action_collection_t col,
                                          uint64_t idx,
                                          PASMP_action_t *action_out,
                                          PASMP_error_descriptor_t *) {
  if (!col || !action_out)
    return PASMP_INVALID_ARGUMENT;
  *action_out = col->actions[idx];
  return PASMP_SUCCESS;
}

PASMP_status_t PASMP_plugin_actions(PASMP_plugin_t plugin,
                                    PASMP_action_collection_t col,
                                    PASMP_error_descriptor_t *err_out) {
  if (!plugin || !col)
    return PASMP_INVALID_ARGUMENT;
  try {
    const auto &p = *reinterpret_cast<plugin::my_plugin *>(plugin);
    auto snapshot = p.snapshot();
    col->actions.reserve(snapshot.size());
    for (const auto &id : snapshot)
      col->actions.push_back(std::bit_cast<PASMP_action_t>(id));
  } catch (const std::bad_alloc &) {
    return alloc_error(err_out, "Error allocating space for actions");
  } catch (const std::exception &e) {
    return generic_error<PASMP_plugin_t>(err_out, e);
  } catch (...) {
    return unknown_error(err_out);
  }
  return PASMP_SUCCESS;
}

#pragma endregion

#pragma region plugin_operations_impl

PASMP_status_t PASMP_plugin_create(PASMP_plugin_t *out,
                                   PASMP_plugin_attr_t attr,
                                   PASMP_error_descriptor_t *err_out) {
  if (!out || !attr)
    return PASMP_INVALID_ARGUMENT;
  try {
    const auto &attributes =
        *reinterpret_cast<plugin::plugin_attributes_t *>(attr);
    *out = reinterpret_cast<PASMP_plugin_t>(
        &plugin::my_plugin::create(attributes));
  } catch (const plugin::invalid_path &e) {
    return path_error(err_out, e);
  } catch (const std::bad_alloc &) {
    return alloc_error(err_out, "Error allocating space for plugin");
  } catch (const std::exception &e) {
    return generic_error<PASMP_plugin_t>(err_out, e);
  } catch (...) {
    return unknown_error(err_out);
  }
  return PASMP_SUCCESS;
}

PASMP_status_t PASMP_plugin_addref(PASMP_plugin_t p) {
  if (!p)
    return PASMP_INVALID_ARGUMENT;
  reinterpret_cast<plugin::my_plugin *>(p)->addref();
  return PASMP_SUCCESS;
}

PASMP_status_t PASMP_plugin_release(PASMP_plugin_t p) {
  if (p)
    reinterpret_cast<plugin::my_plugin *>(p)->release();
  return PASMP_SUCCESS;
}

#pragma endregion

#pragma region plugin_configure_impl

PASMP_status_t PASMP_plugin_configure_gui(PASMP_plugin_t,
                                          PASMP_on_config_finish_t *cb,
                                          void *data,
                                          PASMP_error_descriptor_t *err) {
  fill_error_descriptor(err, "GUI configuration is currently not implemented");
  return PASMP_NOT_IMPLEMENTED;
}

PASMP_status_t PASMP_plugin_configure_cli(PASMP_plugin_t handle,
                                          PASMP_on_config_finish_t *cb,
                                          void *data,
                                          PASMP_error_descriptor_t *err) {
  if (!handle || !cb)
    return PASMP_INVALID_ARGUMENT;
  try {
    auto &p = *reinterpret_cast<plugin::my_plugin *>(handle);
    auto available = p.configure(
        [data, cb](std::exception_ptr eptr, plugin::configuration_status s) {
          if (eptr) {
            PASMP_status_t status = PASMP_UNKNOWN;
            try {
              std::rethrow_exception(eptr);
            } catch (const std::bad_alloc &) {
              status = PASMP_ERROR_ALLOC;
            } catch (const std::logic_error &) {
              status = PASMP_ERROR_FATAL;
            } catch (const std::system_error &) {
              status = PASMP_ERROR_SYSTEM;
            } catch (const std::exception &) {
              status = PASMP_ERROR_OTHER;
            } catch (...) {
              status = PASMP_UNKNOWN;
            }
            cb(status, {}, data);
          }
          switch (s) {
          case plugin::configuration_status::finish:
            cb(PASMP_SUCCESS, PASMP_CONFIG_SUCCESS, data);
            return;
          case plugin::configuration_status::cancel:
            cb(PASMP_SUCCESS, PASMP_CONFIG_CANCEL, data);
            return;
          }
          cb(PASMP_INVALID_STATUS, {}, data);
        });
    if (!available)
      return PASMP_UNAVAILABLE;
  } catch (const std::bad_alloc &) {
    return alloc_error(err, "Error allocating space for configuration");
  } catch (const std::logic_error &e) {
    return fatal_error(err, e);
  } catch (const std::system_error &e) {
    return system_error(err, e);
  } catch (const std::exception &e) {
    return generic_error<PASMP_plugin_t>(err, e);
  } catch (...) {
    return unknown_error(err);
  }
  return PASMP_SUCCESS;
}

#pragma endregion

#pragma region plugin_descriptor_impl

PASMP_status_t
PASMP_plugin_descriptor_create(PASMP_plugin_descriptor_t *out,
                               PASMP_error_descriptor_t *err_out) {
  if (!out)
    return PASMP_INVALID_ARGUMENT;
  try {
    *out = reinterpret_cast<PASMP_plugin_descriptor_t>(
        new plugin::plugin_descriptor_t{});
  } catch (...) {
    return alloc_error(err_out, "Error allocating space for plugin descriptor");
  }
  return PASMP_SUCCESS;
}

PASMP_status_t
PASMP_plugin_descriptor_destroy(PASMP_plugin_descriptor_t descr) {
  if (descr)
    delete reinterpret_cast<plugin::plugin_descriptor_t *>(descr);
  return PASMP_SUCCESS;
}

PASMP_string_view_t PASMP_plugin_name(PASMP_plugin_descriptor_t descr,
                                      int32_t short_variant) {
  auto &d = *reinterpret_cast<const plugin::plugin_descriptor_t *>(descr);
  size_t size = short_variant ? 0 : d.name.size();
  const char *str = short_variant ? nullptr : d.name.c_str();
  return {str, size};
}

PASMP_string_view_t PASMP_plugin_description(PASMP_plugin_descriptor_t descr,
                                             int32_t short_variant) {
  auto &d = *reinterpret_cast<const plugin::plugin_descriptor_t *>(descr);
  size_t size = short_variant ? 0 : d.description.size();
  const char *str = short_variant ? nullptr : d.description.c_str();
  return {str, size};
}

#pragma endregion

#pragma region plugin_attr_impl

PASMP_status_t PASMP_plugin_attr_create(PASMP_plugin_attr_t *out,
                                        PASMP_error_descriptor_t *err_out) {
  if (!out)
    return PASMP_INVALID_ARGUMENT;
  try {
    *out = reinterpret_cast<PASMP_plugin_attr_t>(
        new plugin::plugin_attributes_t{});
  } catch (...) {
    return alloc_error(err_out, "Error allocating space for attributes");
  }
  return PASMP_SUCCESS;
}

PASMP_status_t PASMP_plugin_attr_destroy(PASMP_plugin_attr_t attr) {
  if (attr)
    delete reinterpret_cast<plugin::plugin_attributes_t *>(attr);
  return PASMP_SUCCESS;
}

PASMP_status_t
PASMP_plugin_attr_on_action_mod(PASMP_plugin_attr_t attr,
                                PASMP_on_action_modified_t *cb, void *data,
                                PASMP_error_descriptor_t *err_out) {
  if (!attr || !cb)
    return PASMP_INVALID_ARGUMENT;
  plugin::plugin_attributes_t &a =
      *reinterpret_cast<plugin::plugin_attributes_t *>(attr);
  try {
    a.on_action_modified = [cb, data](plugin::action_id x) {
      cb(std::bit_cast<PASMP_action_t>(x), data);
    };
  } catch (const std::exception &e) {
    return generic_error<PASMP_plugin_attr_t>(err_out, e);
  } catch (...) {
    return unknown_error(err_out);
  }
  return PASMP_SUCCESS;
}

PASMP_status_t
PASMP_plugin_attr_on_action_add(PASMP_plugin_attr_t attr,
                                PASMP_on_action_added_t *cb, void *data,
                                PASMP_error_descriptor_t *err_out) {
  if (!attr || !cb)
    return PASMP_INVALID_ARGUMENT;
  plugin::plugin_attributes_t &a =
      *reinterpret_cast<plugin::plugin_attributes_t *>(attr);
  try {
    a.on_action_added = [cb, data](plugin::action_id x) {
      cb(std::bit_cast<PASMP_action_t>(x), data);
    };
  } catch (const std::exception &e) {
    return generic_error<PASMP_plugin_attr_t>(err_out, e);
  } catch (...) {
    return unknown_error(err_out);
  }
  return PASMP_SUCCESS;
}

PASMP_status_t
PASMP_plugin_attr_on_action_rm(PASMP_plugin_attr_t attr,
                               PASMP_on_action_removed_t *cb, void *data,
                               PASMP_error_descriptor_t *err_out) {
  if (!attr || !cb)
    return PASMP_INVALID_ARGUMENT;
  plugin::plugin_attributes_t &a =
      *reinterpret_cast<plugin::plugin_attributes_t *>(attr);
  try {
    a.on_action_removed = [cb, data](plugin::action_id x) {
      cb(std::bit_cast<PASMP_action_t>(x), data);
    };
  } catch (const std::bad_alloc &) {
    return alloc_error(err_out,
                       "Error allocating space for attribute callback");
  } catch (const std::exception &e) {
    return generic_error<PASMP_plugin_attr_t>(err_out, e);
  } catch (...) {
    return unknown_error(err_out);
  }
  return PASMP_SUCCESS;
}

PASMP_status_t
PASMP_plugin_attr_persistence_path(PASMP_plugin_attr_t attr,
                                   PASMP_string_view_t path,
                                   PASMP_error_descriptor_t *err_out) {
  if (!attr || !path.data || !path.size)
    return PASMP_INVALID_ARGUMENT;
  plugin::plugin_attributes_t &a =
      *reinterpret_cast<plugin::plugin_attributes_t *>(attr);
  try {
    a.persistence_path = std::string_view(path.data, path.size);
  } catch (const std::bad_alloc &) {
    return alloc_error(err_out,
                       "Error allocating space for attribute callback");
  } catch (const std::exception &e) {
    return generic_error<PASMP_plugin_attr_t>(err_out, e);
  } catch (...) {
    return unknown_error(err_out);
  }
  return PASMP_SUCCESS;
}

#pragma endregion

#pragma region action_descriptor_impl

PASMP_status_t PASMP_action_descriptor_create(PASMP_plugin_t plugin,
                                              PASMP_action_t action,
                                              PASMP_action_descriptor_t *descr,
                                              PASMP_error_descriptor_t *err) {
  if (!plugin || !action || !descr)
    return PASMP_INVALID_ARGUMENT;

  try {
    const auto &p = *reinterpret_cast<plugin::my_plugin *>(plugin);
    auto id = std::bit_cast<plugin::action_id>(action);
    *descr = reinterpret_cast<PASMP_action_descriptor_t>(
        new plugin::action_descriptor_t{p.retrieve(id).descriptor()});
    return PASMP_SUCCESS;
  } catch (const plugin::action_does_not_exist &) {
    return PASMP_ERROR_ACTION_NOENT;
  } catch (const std::bad_alloc &) {
    return alloc_error(err, "Error allocating space for action descriptor");
  } catch (const std::exception &e) {
    return generic_error<PASMP_action_t>(err, e);
  } catch (...) {
    return unknown_error(err);
  }
}

PASMP_status_t
PASMP_action_descriptor_destroy(PASMP_action_descriptor_t descr) {
  if (descr)
    delete reinterpret_cast<plugin::action_descriptor_t *>(descr);
  return PASMP_SUCCESS;
}

PASMP_string_view_t PASMP_action_name(PASMP_action_descriptor_t descr,
                                      int32_t short_variant) {
  auto &d = *reinterpret_cast<const plugin::action_descriptor_t *>(descr);
  size_t size = short_variant ? 0 : d.name().size();
  const char *str = short_variant ? nullptr : d.name().c_str();
  return {str, size};
}

PASMP_string_view_t PASMP_action_description(PASMP_action_descriptor_t descr,
                                             int32_t short_variant) {
  auto &d = *reinterpret_cast<const plugin::action_descriptor_t *>(descr);
  size_t size = short_variant ? 0 : d.description().size();
  const char *str = short_variant ? nullptr : d.description().c_str();
  return {str, size};
}

#pragma endregion

#pragma region action_operations_impl

PASMP_status_t PASMP_action_serialize(PASMP_action_t a, char *into,
                                      uint64_t *size_inout,
                                      PASMP_error_descriptor_t *err_out) {
  if (!a || !size_inout)
    return PASMP_INVALID_ARGUMENT;

  // if destination buffer is null, query the required size
  if (!into) {
    *size_inout = 16;
  } else {
    auto id = std::bit_cast<plugin::action_id>(a);
    auto [ptr, ec] = std::to_chars(into, into + *size_inout, id, 16);
    if (auto err = std::make_error_code(ec))
      return serialization_error(err_out, err);
    *size_inout = ptr - into;
  }
  return PASMP_SUCCESS;
}

PASMP_status_t PASMP_action_deserialize(PASMP_action_t *out, const char *from,
                                        uint64_t size,
                                        PASMP_error_descriptor_t *err_out) {
  if (!out || !from)
    return PASMP_INVALID_ARGUMENT;

  plugin::action_id id;
  auto [ptr, ec] = std::from_chars(from, from + size, id, 16);
  if (auto err = std::make_error_code(ec))
    return serialization_error(err_out, err);
  *out = std::bit_cast<PASMP_action_t>(id);
  return PASMP_SUCCESS;
}

PASMP_status_t PASMP_action_destroy(PASMP_action_t) { return PASMP_SUCCESS; }

PASMP_status_t PASMP_action_execute(PASMP_plugin_t p, PASMP_action_t a,
                                    PASMP_payload_t payload,
                                    PASMP_error_descriptor_t *err_out) {

  if (!p || !a)
    return PASMP_INVALID_ARGUMENT;
  if (payload.tag != PASMP_PAYLOAD_INT32) {
    // TODO maybe custom message
    return PASMP_ERROR_PAYLOAD_INVALID;
  }
  try {
    auto &plug = *reinterpret_cast<plugin::my_plugin *>(p);
    auto id = std::bit_cast<plugin::action_id>(a);
    plug.execute(id, payload.data->int32_value);
  } catch (const plugin::action_does_not_exist &) {
    return PASMP_ERROR_ACTION_NOENT;
  } catch (const plugin::action_error &e) {
    fill_error_descriptor(err_out, e.what());
    return PASMP_ERROR_ACTION_EXEC;
  } catch (const std::exception &e) {
    return generic_error<PASMP_action_t>(err_out, e);
  } catch (...) {
    return unknown_error(err_out);
  }
  return PASMP_SUCCESS;
}

PASMP_status_t PASMP_action_execute_async(PASMP_plugin_t p, PASMP_action_t a,
                                          PASMP_payload_t payload,
                                          PASMP_on_action_finish_t *cb,
                                          void *data,
                                          PASMP_error_descriptor_t *err_out) {
  if (!p || !a)
    return PASMP_INVALID_ARGUMENT;
  if (payload.tag != PASMP_PAYLOAD_INT32) {
    fill_error_descriptor(err_out, "Payload must be a 32-bit signed integer");
    return PASMP_ERROR_PAYLOAD_INVALID;
  }
  auto size = err_out->size;
  auto status = PASMP_SUCCESS;
  try {
    auto &plug = *reinterpret_cast<plugin::my_plugin *>(p);
    auto id = std::bit_cast<plugin::action_id>(a);
    plug.execute(id, payload.data->int32_value);
  } catch (const plugin::action_does_not_exist &) {
    status = PASMP_ERROR_ACTION_NOENT;
  } catch (const plugin::action_error &e) {
    fill_error_descriptor(err_out, e.what());
    status = PASMP_ERROR_ACTION_EXEC;
  } catch (const std::exception &e) {
    status = generic_error<PASMP_action_t>(err_out, e);
  } catch (...) {
    status = unknown_error(err_out);
  }
  cb(status, {.data = err_out->what, .size = size - err_out->size}, data);
  return status;
}

uint64_t PASMP_action_hash(PASMP_action_t a) {
  auto id = std::bit_cast<plugin::action_id>(a);
  return std::hash<decltype(id)>{}(id);
}

int32_t PASMP_action_equal(PASMP_action_t lhs, PASMP_action_t rhs) {
  auto id_lhs = std::bit_cast<plugin::action_id>(lhs);
  auto id_rhs = std::bit_cast<plugin::action_id>(rhs);
  return id_lhs == id_rhs;
}

#pragma endregion

#pragma region version_impl

PASMP_status_t PASMP_version_create(PASMP_version_t *out,
                                    PASMP_error_descriptor_t *err_out) {
  if (!out)
    return PASMP_INVALID_ARGUMENT;
  try {
    *out = reinterpret_cast<PASMP_version_t>(new plugin::plugin_version_t{});
    return PASMP_SUCCESS;
  } catch (...) {
    return alloc_error(err_out, "Error allocating space for version");
  }
}

PASMP_status_t PASMP_version_destroy(PASMP_version_t v) {
  if (v)
    delete reinterpret_cast<plugin::plugin_version_t *>(v);
  return PASMP_SUCCESS;
}

uint16_t PASMP_version_major(PASMP_version_t v) {
  return reinterpret_cast<const plugin::plugin_version_t *>(v)->major;
}

uint16_t PASMP_version_minor(PASMP_version_t v) {
  return reinterpret_cast<const plugin::plugin_version_t *>(v)->minor;
}

uint16_t PASMP_version_patch(PASMP_version_t v) {
  return reinterpret_cast<const plugin::plugin_version_t *>(v)->patch;
}

PASMP_string_view_t PASMP_version_pre(PASMP_version_t v) {
  auto &ver = *reinterpret_cast<const plugin::plugin_version_t *>(v);
  return {ver.pre_release.c_str(), ver.pre_release.size()};
}

PASMP_string_view_t PASMP_version_build(PASMP_version_t v) {
  auto &ver = *reinterpret_cast<const plugin::plugin_version_t *>(v);
  return {ver.build.c_str(), ver.build.size()};
}

#pragma endregion

#pragma region misc_impl

uint32_t PASMP_version() { return PASMP_VERSION; }

int32_t PASMP_is_library_compatible(uint32_t ver) {

  uint32_t major = ver >> 16;
  return major == PASMP_VERSION_MAJOR;
}

#pragma endregion
