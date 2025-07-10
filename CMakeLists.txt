cmake_minimum_required(VERSION 3.12.0)
project(IsleAutorun C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS on)

set(BUILD_SHARED_LIBS off)
set(BUILD_STATIC_LIBS on)
add_subdirectory("thirdparty")

find_package(SDL3 CONFIG REQUIRED)

add_executable(autorun
    src/assets.cpp
    src/main.cpp
)

target_link_libraries(autorun
    SDL3
    libsmacker
    iniparser-static
)
