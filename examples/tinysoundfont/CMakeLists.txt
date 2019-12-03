cmake_minimum_required(VERSION 2.8.12)

project(TinySoundFontPlay)

add_executable(tsplay
    fluid_seq.cpp
    fluid_play.cpp
)
target_include_directories(tsplay PRIVATE
    ${TinySoundFontPlay_SOURCE_DIR}/../../include
)
target_link_libraries(tsplay SDL2)

