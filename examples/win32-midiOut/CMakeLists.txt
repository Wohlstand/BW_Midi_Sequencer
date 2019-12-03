cmake_minimum_required(VERSION 2.8.12)

project(w32MidiPlay)

add_executable(w32playmidi
    w32-playmidi.c
    native_midi_win32_seq.cpp
)

target_include_directories(w32playmidi PRIVATE
    ${w32MidiPlay_SOURCE_DIR}/../../include
)

target_link_libraries(fplay PRIVATE -lwinmm)
if(NOT MSVC)
    target_link_libraries(fplay PRIVATE -lstdc++)
endif()

