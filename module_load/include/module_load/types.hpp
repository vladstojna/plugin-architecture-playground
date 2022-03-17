#pragma once

#include <module_load/traits.hpp>

#include <plugin/plugin_interface.h>

namespace modl {

struct version_tr : detail::module_function_traits<PASMP_version> {
  static constexpr char name[] = "PASMP_version";
};

struct is_compatible_tr
    : detail::module_function_traits<PASMP_is_library_compatible> {
  static constexpr char name[] = "PASMP_is_library_compatible";
};

struct version_create_tr
    : detail::module_function_traits<PASMP_version_create> {
  static constexpr char name[] = "PASMP_version_create";
};

struct version_destroy_tr
    : detail::module_function_traits<PASMP_version_destroy> {
  static constexpr char name[] = "PASMP_version_destroy";
};

struct version_major_tr : detail::module_function_traits<PASMP_version_major> {
  static constexpr char name[] = "PASMP_version_major";
};

struct version_minor_tr : detail::module_function_traits<PASMP_version_minor> {
  static constexpr char name[] = "PASMP_version_minor";
};

struct version_patch_tr : detail::module_function_traits<PASMP_version_patch> {
  static constexpr char name[] = "PASMP_version_patch";
};

struct version_pre_tr : detail::module_function_traits<PASMP_version_pre> {
  static constexpr char name[] = "PASMP_version_pre";
};

struct version_build_tr : detail::module_function_traits<PASMP_version_build> {
  static constexpr char name[] = "PASMP_version_build";
};

struct plugin_attr_create_tr
    : detail::module_function_traits<PASMP_plugin_attr_create> {
  static constexpr char name[] = "PASMP_plugin_attr_create";
};

struct plugin_attr_destroy_tr
    : detail::module_function_traits<PASMP_plugin_attr_destroy> {
  static constexpr char name[] = "PASMP_plugin_attr_destroy";
};

struct plugin_attr_on_action_mod_tr
    : detail::module_function_traits<PASMP_plugin_attr_on_action_mod> {
  static constexpr char name[] = "PASMP_plugin_attr_on_action_mod";
};

struct plugin_attr_on_action_add_tr
    : detail::module_function_traits<PASMP_plugin_attr_on_action_add> {
  static constexpr char name[] = "PASMP_plugin_attr_on_action_add";
};

struct plugin_attr_on_action_rm_tr
    : detail::module_function_traits<PASMP_plugin_attr_on_action_rm> {
  static constexpr char name[] = "PASMP_plugin_attr_on_action_rm";
};

struct plugin_attr_persistence_path_tr
    : detail::module_function_traits<PASMP_plugin_attr_persistence_path> {
  static constexpr char name[] = "PASMP_plugin_attr_persistence_path";
};

struct plugin_create_tr : detail::module_function_traits<PASMP_plugin_create> {
  static constexpr char name[] = "PASMP_plugin_create";
};

struct plugin_addref_tr : detail::module_function_traits<PASMP_plugin_addref> {
  static constexpr char name[] = "PASMP_plugin_addref";
};

struct plugin_release_tr
    : detail::module_function_traits<PASMP_plugin_release> {
  static constexpr char name[] = "PASMP_plugin_release";
};

struct plugin_descriptor_create_tr
    : detail::module_function_traits<PASMP_plugin_descriptor_create> {
  static constexpr char name[] = "PASMP_plugin_descriptor_create";
};

struct plugin_descriptor_destroy_tr
    : detail::module_function_traits<PASMP_plugin_descriptor_destroy> {
  static constexpr char name[] = "PASMP_plugin_descriptor_destroy";
};

struct plugin_name_tr : detail::module_function_traits<PASMP_plugin_name> {
  static constexpr char name[] = "PASMP_plugin_name";
};

struct plugin_description_tr
    : detail::module_function_traits<PASMP_plugin_description> {
  static constexpr char name[] = "PASMP_plugin_description";
};

struct action_collection_create_tr
    : detail::module_function_traits<PASMP_action_collection_create> {
  static constexpr char name[] = "PASMP_action_collection_create";
};

struct action_collection_destroy_tr
    : detail::module_function_traits<PASMP_action_collection_destroy> {
  static constexpr char name[] = "PASMP_action_collection_destroy";
};

struct action_collection_size_tr
    : detail::module_function_traits<PASMP_action_collection_size> {
  static constexpr char name[] = "PASMP_action_collection_size";
};

struct action_collection_at_tr
    : detail::module_function_traits<PASMP_action_collection_at> {
  static constexpr char name[] = "PASMP_action_collection_at";
};

struct plugin_actions_tr
    : detail::module_function_traits<PASMP_plugin_actions> {
  static constexpr char name[] = "PASMP_plugin_actions";
};

struct plugin_configure_gui_tr
    : detail::module_function_traits<PASMP_plugin_configure_gui> {
  static constexpr char name[] = "PASMP_plugin_configure_gui";
};

struct plugin_configure_cli_tr
    : detail::module_function_traits<PASMP_plugin_configure_cli> {
  static constexpr char name[] = "PASMP_plugin_configure_cli";
};

struct action_serialize_tr
    : detail::module_function_traits<PASMP_action_serialize> {
  static constexpr char name[] = "PASMP_action_serialize";
};

struct action_deserialize_tr
    : detail::module_function_traits<PASMP_action_deserialize> {
  static constexpr char name[] = "PASMP_action_deserialize";
};

struct action_destroy_tr
    : detail::module_function_traits<PASMP_action_destroy> {
  static constexpr char name[] = "PASMP_action_destroy";
};

struct action_descriptor_create_tr
    : detail::module_function_traits<PASMP_action_descriptor_create> {
  static constexpr char name[] = "PASMP_action_descriptor_create";
};

struct action_descriptor_destroy_tr
    : detail::module_function_traits<PASMP_action_descriptor_destroy> {
  static constexpr char name[] = "PASMP_action_descriptor_destroy";
};

struct action_name_tr : detail::module_function_traits<PASMP_action_name> {
  static constexpr char name[] = "PASMP_action_name";
};

struct action_description_tr
    : detail::module_function_traits<PASMP_action_description> {
  static constexpr char name[] = "PASMP_plugin_description";
};

struct action_execute_tr
    : detail::module_function_traits<PASMP_action_execute> {
  static constexpr char name[] = "PASMP_action_execute";
};

struct action_execute_async_tr
    : detail::module_function_traits<PASMP_action_execute_async> {
  static constexpr char name[] = "PASMP_action_execute_async";
};

struct action_hash_tr : detail::module_function_traits<PASMP_action_hash> {
  static constexpr char name[] = "PASMP_action_hash";
};

struct action_equal_tr : detail::module_function_traits<PASMP_action_equal> {
  static constexpr char name[] = "PASMP_action_equal";
};

using version_t = module_function<version_tr>;
using is_compatible_t = module_function<is_compatible_tr>;

using version_create_t = module_function<version_create_tr>;
using version_destroy_t = module_function<version_destroy_tr>;
using version_major_t = module_function<version_major_tr>;
using version_minor_t = module_function<version_minor_tr>;
using version_patch_t = module_function<version_patch_tr>;
using version_pre_t = module_function<version_pre_tr>;
using version_build_t = module_function<version_build_tr>;

using plugin_attr_create_t = module_function<plugin_attr_create_tr>;
using plugin_attr_destroy_t = module_function<plugin_attr_destroy_tr>;
using plugin_attr_on_action_mod_t =
    module_function<plugin_attr_on_action_mod_tr>;
using plugin_attr_on_action_add_t =
    module_function<plugin_attr_on_action_add_tr>;
using plugin_attr_on_action_rm_t = module_function<plugin_attr_on_action_rm_tr>;
using plugin_attr_persistence_path_t =
    module_function<plugin_attr_persistence_path_tr>;

using plugin_create_t = module_function<plugin_create_tr>;
using plugin_addref_t = module_function<plugin_addref_tr>;
using plugin_release_t = module_function<plugin_release_tr>;

using plugin_descriptor_create_t = module_function<plugin_descriptor_create_tr>;
using plugin_descriptor_destroy_t =
    module_function<plugin_descriptor_destroy_tr>;
using plugin_name_t = module_function<plugin_name_tr>;
using plugin_description_t = module_function<plugin_description_tr>;

using action_collection_create_t = module_function<action_collection_create_tr>;
using action_collection_destroy_t =
    module_function<action_collection_destroy_tr>;
using action_collection_size_t = module_function<action_collection_size_tr>;
using action_collection_at_t = module_function<action_collection_at_tr>;
using plugin_actions_t = module_function<plugin_actions_tr>;

using plugin_configure_gui_t = module_function<plugin_configure_gui_tr>;
using plugin_configure_cli_t = module_function<plugin_configure_cli_tr>;

using action_serialize_t = module_function<action_serialize_tr>;
using action_deserialize_t = module_function<action_deserialize_tr>;
using action_destroy_t = module_function<action_destroy_tr>;
using action_descriptor_create_t = module_function<action_descriptor_create_tr>;
using action_descriptor_destroy_t =
    module_function<action_descriptor_destroy_tr>;
using action_name_t = module_function<action_name_tr>;
using action_description_t = module_function<action_description_tr>;
using action_execute_t = module_function<action_execute_tr>;
using action_execute_async_t = module_function<action_execute_async_tr>;
using action_hash_t = module_function<action_hash_tr>;
using action_equal_t = module_function<action_equal_tr>;

} // namespace modl
