#pragma once

#include <module_load/types.hpp>
#include <module_load/version.hpp>

#include <filesystem>
#include <memory>
#include <string>

namespace modl {

class loaded_module {
private:
  struct impl;
  std::shared_ptr<const impl> handle_;

public:
  struct functions {
    version_create_t version_create;
    version_destroy_t version_destroy;
    version_major_t version_major;
    version_minor_t version_minor;
    version_patch_t version_patch;
    version_pre_t version_pre;
    version_build_t version_build;

    plugin_attr_create_t plugin_attr_create;
    plugin_attr_destroy_t plugin_attr_destroy;
    plugin_attr_on_action_mod_t plugin_attr_on_action_mod;
    plugin_attr_on_action_add_t plugin_attr_on_action_add;
    plugin_attr_on_action_rm_t plugin_attr_on_action_rm;
    plugin_attr_persistence_path_t plugin_attr_persistence_path;

    plugin_create_t plugin_create;
    plugin_addref_t plugin_addref;
    plugin_release_t plugin_release;

    plugin_descriptor_create_t plugin_descriptor_create;
    plugin_descriptor_destroy_t plugin_descriptor_destroy;
    plugin_name_t plugin_name;
    plugin_description_t plugin_description;

    action_collection_create_t action_collection_create;
    action_collection_destroy_t action_collection_destroy;
    action_collection_size_t action_collection_size;
    action_collection_at_t action_collection_at;
    plugin_actions_t plugin_actions;

    plugin_configure_gui_t plugin_configure_gui;
    plugin_configure_cli_t plugin_configure_cli;

    action_serialize_t action_serialize;
    action_deserialize_t action_deserialize;
    action_destroy_t action_destroy;
    action_descriptor_create_t action_descriptor_create;
    action_descriptor_destroy_t action_descriptor_destroy;
    action_name_t action_name;
    action_description_t action_description;
    action_execute_t action_execute;
    action_execute_async_t action_execute_async;
    action_hash_t action_hash;
    action_equal_t action_equal;

    explicit functions(const impl &);
  };

  explicit loaded_module(const std::filesystem::path &);

  library_version version() const noexcept;
  const std::string &path() const noexcept;
  const std::string &filename() const noexcept;
  const functions &funcs() const noexcept;
};

bool operator==(const loaded_module &, const loaded_module &) noexcept;

} // namespace modl
