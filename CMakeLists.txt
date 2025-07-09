cmake_minimum_required(VERSION 3.12.0)
project(IsleAutorun C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS on)

find_package(SDL3 CONFIG REQUIRED)

add_executable(autorun
    src/main.cpp
)

target_link_libraries(autorun
    SDL3
)
