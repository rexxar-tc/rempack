cmake_minimum_required(VERSION 3.21)

#set your project name here. this will be used throughout the build system
#this is also the name of the compiled binary
project(rempack)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(extern/rM2-stuff)
add_subdirectory(opkg)

include_directories("${CMAKE_SOURCE_DIR}/include")
include_directories("${CMAKE_SOURCE_DIR}/assets")
include_directories("${CMAKE_SOURCE_DIR}/ui")
include_directories("${CMAKE_SOURCE_DIR}")
include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/extern")
include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/extern/libs")


add_executable(${PROJECT_NAME} src/main.cpp
        src/Rempack.cpp
        src/Rempack.h)

#target_compile_options(${PROJECT_NAME}
#        PRIVATE $<$<CONFIG:Debug>:-g -Wall -Wno-unused-variable>
#        PRIVATE $<$<CONFIG:Release>:-O3>
#)
target_link_libraries(${PROJECT_NAME}
        PRIVATE
        rMlib
        opkg)

