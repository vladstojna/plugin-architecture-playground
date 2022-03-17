cmake_minimum_required (VERSION 3.12)

project ("MyMidi")

add_library(MyMidi STATIC
    "src/error.cpp"
    "include/midi/error.hpp"
    "include/midi/message.hpp"
    "include/midi/midi_in.hpp"
    "src/midi_in.cpp"
    "src/wincommon.h"
    "include/midi/midi.hpp"
 "include/midi/midiin_attributes.hpp" "include/midi/passkey.hpp" "src/message.cpp")

target_compile_features(MyMidi PRIVATE cxx_std_20)
target_include_directories(MyMidi PUBLIC include)
target_include_directories(MyMidi PUBLIC ../include)
