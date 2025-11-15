cmake_minimum_required(VERSION 2.8.12...4.0)

project(MidiDump)

add_executable(mididump mididump.cpp)

target_include_directories(mididump PRIVATE
    ${MidiDump_SOURCE_DIR}/../../include
)

target_compile_definitions(mididump PRIVATE BWMIDI_ENABLE_DEBUG_SONG_DUMP BWMIDI_ENABLE_OPL_MUSIC_SUPPORT)
