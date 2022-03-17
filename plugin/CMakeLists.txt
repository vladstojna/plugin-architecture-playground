cmake_minimum_required (VERSION 3.12)

project ("MyPlugin")

find_package(fmt CONFIG REQUIRED)

add_library(MyPlugin MODULE "plugin_impl.cpp" "plugin_impl.hpp" "plugin_interface.cpp"  "../include/plugin/plugin_interface.h")
add_library(MyPlugin2 MODULE "plugin_impl.cpp" "plugin_impl.hpp" "plugin_interface.cpp"  "../include/plugin/plugin_interface.h")
add_library(MyPlugin3 MODULE "plugin_impl.cpp" "plugin_impl.hpp" "plugin_interface.cpp"  "../include/plugin/plugin_interface.h")

target_compile_features(MyPlugin PRIVATE cxx_std_20)
target_compile_features(MyPlugin2 PRIVATE cxx_std_20)
target_compile_features(MyPlugin3 PRIVATE cxx_std_20)
target_include_directories(MyPlugin PRIVATE ../include)
target_include_directories(MyPlugin2 PRIVATE ../include)
target_include_directories(MyPlugin3 PRIVATE ../include)

target_link_libraries(MyPlugin PRIVATE fmt::fmt-header-only)
target_link_libraries(MyPlugin2 PRIVATE fmt::fmt-header-only)
target_link_libraries(MyPlugin3 PRIVATE fmt::fmt-header-only)
