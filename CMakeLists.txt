cmake_minimum_required (VERSION 3.12)

project ("PluginArchitectureSample")

find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

add_executable (PluginArchitectureSample
 "src/PluginArchitectureSample.cpp"
 "src/blocking_queue.hpp"
 "src/misc.hpp"
 "src/misc.cpp"
)

add_subdirectory(plugin)
add_subdirectory(wrapper)
add_subdirectory(module_load)
add_subdirectory(midi)

target_compile_features(PluginArchitectureSample PRIVATE cxx_std_20)
target_include_directories(PluginArchitectureSample PRIVATE include)
target_include_directories(PluginArchitectureSample PRIVATE wrapper/include)

add_dependencies(PluginArchitectureSample MyPlugin)
target_link_libraries(PluginArchitectureSample PRIVATE MyWrapper)
target_link_libraries(PluginArchitectureSample PRIVATE MyMidi)
target_link_libraries(PluginArchitectureSample PRIVATE fmt::fmt-header-only)
target_link_libraries(PluginArchitectureSample PRIVATE spdlog::spdlog spdlog::spdlog_header_only)
