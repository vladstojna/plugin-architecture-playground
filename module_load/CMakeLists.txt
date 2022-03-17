cmake_minimum_required (VERSION 3.12)

project ("MyModuleLoad")

add_library(MyModuleLoad STATIC
    "include/module_load/exception.hpp"
    "include/module_load/module.hpp"
    "include/module_load/modulefwd.hpp"
    "include/module_load/traits.hpp"
    "include/module_load/types.hpp"
    "include/module_load/version.hpp"
    "src/exception.cpp"
    "src/module.cpp"
)

target_compile_features(MyModuleLoad PRIVATE cxx_std_20)

target_include_directories(MyModuleLoad PRIVATE ../include)
target_include_directories(MyModuleLoad PRIVATE include)
