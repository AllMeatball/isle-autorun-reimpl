# Thanks isle-portable! (https://github.com/isledecomp/isle-portable/blob/master/3rdparty/CMakeLists.txt)
set(libsmacker_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libsmacker")
add_library(libsmacker STATIC
    ${libsmacker_SOURCE_DIR}/smacker.c
)
target_include_directories(libsmacker PUBLIC ${libsmacker_SOURCE_DIR})

add_subdirectory("iniparser")
