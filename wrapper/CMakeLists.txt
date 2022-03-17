cmake_minimum_required (VERSION 3.12)

project ("MyWrapper")

add_library(MyWrapper STATIC
    "src/action_descriptor.cpp"
    "src/action.cpp"
    "src/error.cpp"
    "src/error_descriptor.cpp"
    "src/payload.cpp"
    "src/plugin.cpp"
    "src/plugin_configurator.cpp"
    "src/plugin_descriptor.cpp"
    "src/plugin_version.cpp"
    "src/rcstring.cpp"
    "include/wrap/action_descriptor.hpp"
    "include/wrap/action.hpp"
    "include/wrap/basic_descriptor.hpp"
    "include/wrap/error.hpp"
    "include/wrap/error_descriptor.hpp"
    "include/wrap/nothrow.hpp"
    "include/wrap/passkey.hpp"
    "include/wrap/payload.hpp"
    "include/wrap/plugin.hpp"
    "include/wrap/plugin_configurator.hpp"
    "include/wrap/plugin_descriptor.hpp"
    "include/wrap/plugin_object.hpp"
    "include/wrap/plugin_version.hpp"
    "include/wrap/rcstring.hpp"
    "include/wrap/visibility.hpp"
    "include/wrap/wrap.hpp"
    "src/status_utils.hpp"
    "src/status_utils.cpp"
    "include/wrap/plugin_attributes.hpp"
    "include/wrap/action_event.hpp"
    "include/wrap/action_executor.hpp"
    "src/action_executor.cpp"
)

target_compile_features(MyWrapper PRIVATE cxx_std_20)
target_compile_definitions(MyWrapper PRIVATE WRAPPER_BUILDING_LIB)

target_include_directories(MyWrapper PRIVATE ../include)
target_include_directories(MyWrapper PUBLIC ../module_load/include)
target_include_directories(MyWrapper PUBLIC include)

target_link_libraries(MyWrapper PUBLIC MyModuleLoad)
