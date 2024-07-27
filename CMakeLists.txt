cmake_minimum_required(VERSION 3.5.0)
project(PIN-8 VERSION 0.1.0 LANGUAGES C CXX)

add_executable(PIN-8 src/main.cpp)

add_subdirectory(external/raylib)
include_directories("external/raylib/src")
target_link_libraries(PIN-8 raylib)