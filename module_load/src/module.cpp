#include <module_load/exception.hpp>
#include <module_load/module.hpp>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <libloaderapi.h>

#include <cassert>
#include <iostream>

namespace {

namespace detail {

template <typename T> using type_t = T::type;
template <typename T> using pointer_type_t = T::pointer_type;
template <typename T> using name_t = decltype(T::name);

template <typename T, template <typename> typename, typename = void>
struct has_member : std::false_type {};

template <typename T, template <typename> typename Op>
struct has_member<T, Op, std::void_t<Op<T>>> : std::true_type {};

template <typename, typename = void>
struct has_type_member : std::false_type {};
template <typename T>
struct has_type_member<T, std::void_t<type_t<T>>> : std::true_type {};

template <typename, typename = void>
struct has_pointer_type_member : std::false_type {};
template <typename T>
struct has_pointer_type_member<T, std::void_t<pointer_type_t<T>>>
    : std::true_type {};

template <typename, typename = void>
struct has_name_member : std::false_type {};
template <typename T>
struct has_name_member<T, std::void_t<name_t<T>>> : std::true_type {};

template <typename T>
using is_module_function_traits =
    std::conjunction<has_type_member<T>, has_pointer_type_member<T>,
                     has_name_member<T>>;

template <typename T>
constexpr auto is_module_function_traits_v =
    is_module_function_traits<T>::value;

} // namespace detail

std::error_code get_last_error() noexcept {
  return {static_cast<int>(GetLastError()), std::system_category()};
}

} // namespace

namespace modl {

struct loaded_module::impl {
  using module_t = std::remove_pointer_t<HMODULE>;

  module_t *module_ = nullptr;
  std::string path_;
  std::string filename_;
  library_version version_;
  loaded_module::functions funcs_;

  explicit impl(const std::filesystem::path &path)
      : module_(load_library(path)),
        path_(std::filesystem::relative(path).string()),
        filename_(path.filename().string()), version_(get_version()),
        funcs_(*this) {}

  ~impl() {
    if (BOOL res; module_ && !(res = FreeLibrary(module_))) {
      assert(false);
      std::cerr << "Error freeing library: " << get_last_error();
    }
  }

  library_version get_version() const {
    uint32_t ver = load_function<version_tr>()();
    library_version found = {.major = ((ver >> 16) && 0xffff),
                             .minor = (ver && 0xffff)};
    if (!load_function<is_compatible_tr>()(PASMP_VERSION)) {
      library_version current = {.major = ((PASMP_VERSION >> 16) && 0xffff),
                                 .minor = (PASMP_VERSION && 0xffff)};
      throw module_incompatible(current, found);
    }
    return found;
  }

  static module_t *load_library(const std::filesystem::path &path) {
    module_t *mod = LoadLibraryW(path.native().c_str());
    if (!mod)
      throw std::system_error(get_last_error(), "Error loading library");
    return mod;
  }

  template <typename Traits, typename = std::enable_if_t<
                                 ::detail::is_module_function_traits_v<Traits>>>
  typename Traits::pointer_type load_function() const {
    using pointer = typename Traits::pointer_type;
    auto func = reinterpret_cast<pointer>(
        reinterpret_cast<intptr_t>((GetProcAddress(module_, Traits::name))));
    if (func)
      return func;
    throw function_load_error(get_last_error(), Traits::name);
  }
};

loaded_module::loaded_module(const std::filesystem::path &p)
    : handle_(std::make_shared<loaded_module::impl>(p)) {}

const std::string &loaded_module::path() const noexcept {
  return handle_->path_;
}

const std::string &loaded_module::filename() const noexcept {
  return handle_->filename_;
}

const loaded_module::functions &loaded_module::funcs() const noexcept {
  return handle_->funcs_;
}

library_version loaded_module::version() const noexcept {
  return handle_->version_;
}

loaded_module::functions::functions(const impl &h)
    : version_create{h.load_function<decltype(version_create)::traits>()},
      version_destroy{h.load_function<decltype(version_destroy)::traits>()},
      version_major{h.load_function<decltype(version_major)::traits>()},
      version_minor{h.load_function<decltype(version_minor)::traits>()},
      version_patch{h.load_function<decltype(version_patch)::traits>()},
      version_pre{h.load_function<decltype(version_pre)::traits>()},
      version_build{h.load_function<decltype(version_build)::traits>()},
      plugin_attr_create{
          h.load_function<decltype(plugin_attr_create)::traits>()},
      plugin_attr_destroy{
          h.load_function<decltype(plugin_attr_destroy)::traits>()},
      plugin_attr_on_action_mod{
          h.load_function<decltype(plugin_attr_on_action_mod)::traits>()},
      plugin_attr_on_action_add{
          h.load_function<decltype(plugin_attr_on_action_add)::traits>()},
      plugin_attr_on_action_rm{
          h.load_function<decltype(plugin_attr_on_action_rm)::traits>()},
      plugin_attr_persistence_path{
          h.load_function<decltype(plugin_attr_persistence_path)::traits>()},
      plugin_create{h.load_function<decltype(plugin_create)::traits>()},
      plugin_addref{h.load_function<decltype(plugin_addref)::traits>()},
      plugin_release{h.load_function<decltype(plugin_release)::traits>()},
      plugin_descriptor_create{
          h.load_function<decltype(plugin_descriptor_create)::traits>()},
      plugin_descriptor_destroy{
          h.load_function<decltype(plugin_descriptor_destroy)::traits>()},
      plugin_name{h.load_function<decltype(plugin_name)::traits>()},
      plugin_description{
          h.load_function<decltype(plugin_description)::traits>()},
      action_collection_create{
          h.load_function<decltype(action_collection_create)::traits>()},
      action_collection_destroy{
          h.load_function<decltype(action_collection_destroy)::traits>()},
      action_collection_size{
          h.load_function<decltype(action_collection_size)::traits>()},
      action_collection_at{
          h.load_function<decltype(action_collection_at)::traits>()},
      plugin_actions{h.load_function<decltype(plugin_actions)::traits>()},
      plugin_configure_gui{
          h.load_function<decltype(plugin_configure_gui)::traits>()},
      plugin_configure_cli{
          h.load_function<decltype(plugin_configure_cli)::traits>()},
      action_serialize{h.load_function<decltype(action_serialize)::traits>()},
      action_deserialize{
          h.load_function<decltype(action_deserialize)::traits>()},
      action_destroy{h.load_function<decltype(action_destroy)::traits>()},
      action_descriptor_create{
          h.load_function<decltype(action_descriptor_create)::traits>()},
      action_descriptor_destroy{
          h.load_function<decltype(action_descriptor_destroy)::traits>()},
      action_name{h.load_function<decltype(action_name)::traits>()},
      action_description{
          h.load_function<decltype(action_description)::traits>()},
      action_execute{h.load_function<decltype(action_execute)::traits>()},
      action_execute_async{
          h.load_function<decltype(action_execute_async)::traits>()},
      action_hash{h.load_function<decltype(action_hash)::traits>()},
      action_equal{h.load_function<decltype(action_equal)::traits>()} {}

bool operator==(const loaded_module &lhs, const loaded_module &rhs) noexcept {
  return lhs.path() == rhs.path();
}

} // namespace modl
