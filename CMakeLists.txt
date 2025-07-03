cmake_minimum_required(VERSION 3.21)

#set your project name here. this will be used throughout the build system
#this is also the name of the compiled binary
project(rempack)

set(CMAKE_CXX_STANDARD 17)

#user@host same as an SSH command
#host string should be an IP address or a hostname, if configured
#you can also simply set this variable as an argument passed to CMake: -DREMARKABLE_HOST="192.168.1.102"
if(NOT REMARKABLE_HOST)
    set(REMARKABLE_HOST "remarkable")
endif()

#automatically download rmkit.h
set(RMKIT_DIR "${CMAKE_BINARY_DIR}/include/rmkit")
set(HASH_FILE "${RMKIT_DIR}/sha256sum.txt")
set(RMKIT_FILE "${RMKIT_DIR}/rmkit.h")

#use this if you're building against a custom version of rmkit.h
#set(RMKIT_EXTERN "${CMAKE_SOURCE_DIR}/extern/rmkit.h")

if(NOT DEFINED RMKIT_EXTERN)
    #don't hit the server if we already have the file
    if(NOT EXISTS ${RMKIT_FILE})
        file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/include/rmkit)
        file(DOWNLOAD https://build.rmkit.dev/master/latest/rm/rmkit.h "${RMKIT_FILE}")
        #verify downloaded file against posted hashes
        file(DOWNLOAD https://build.rmkit.dev/master/latest/rm/sha256sum.txt "${HASH_FILE}")
        file(READ "${HASH_FILE}" HASH_LINES)
        #extract the hash from the line ending with rmkit.h
        string(REGEX MATCH "([A-Fa-f0-9]+)[\\t\\ ]+[a-zA-Z0-9/\\]+rmkit\\.h" _ "${HASH_LINES}")
        set(HASH_VALUE "${CMAKE_MATCH_1}")
        #check the hashes
        file(READ "${RMKIT_FILE}" RMKIT_CONTENT)
        string(SHA256 RMKIT_CONTENT_HASH "${RMKIT_CONTENT}")
        file(REMOVE ${HASH_FILE})

        if(NOT "${RMKIT_CONTENT_HASH}" STREQUAL "${HASH_VALUE}")
            message(FATAL_ERROR "Hash verification failed for rmkit.h!")
            file(REMOVE ${RKIT_FILE})
        else()
            message("Hash verification successful for rmkit.h!")
        endif()
    endif()
elseif(EXISTS ${RMKIT_EXTERN})
    set(RMKIT_FILE "${RMKIT_EXTERN}")
    message(NOTICE "Using external rmkit header at ${RMKIT_FILE}")
    include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/extern")
else()
    message(FATAL_ERROR "Could not load rmkit.h! Build cannot proceed! Path: ${RMKIT_EXTERN}")
endif()
#add rmkit.h to our system header search paths
include_directories(SYSTEM "${RMKIT_DIR}")

#add /include to search paths for font_embed.h
include_directories("${CMAKE_SOURCE_DIR}/include")
include_directories("${CMAKE_SOURCE_DIR}/assets")
include_directories("${CMAKE_SOURCE_DIR}/ui")
add_subdirectory("${CMAKE_SOURCE_DIR}/ui")
add_subdirectory("${CMAKE_SOURCE_DIR}/opkg")

add_executable(${PROJECT_NAME} src/main.cpp
        src/rempack.cpp
        #"$<IF:$<BOOL:${RMKIT_EXTERN}>,${RMKIT_FILE},extern/rmkit.h>"
        #${RMKIT_FILE}
        assets/icons/icons_embed.h
        ${UI_SOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE ${UI_INCLUDE_DIRS})
set(CMAKE_CXX_FLAGS_DEBUG "-g3 -Og -fno-omit-frame-pointer -fno-inline -fno-inline-functions-called-once -fno-optimize-sibling-calls")
#target_compile_options(${PROJECT_NAME}
#        PUBLIC $<$<CONFIG:Debug>:-g -O0 -Wall -Wno-unused-variable>
#        #PUBLIC $<$<CONFIG:Debug>:-g3 -Og -Wall -Wno-unused-variable -fno-omit-frame-pointer -fno-inline -fno-inline-functions-called-once -fno-optimize-sibling-calls>
#        PUBLIC $<$<CONFIG:Release>:-O3>
#)
#target_link_options(${PROJECT_NAME}        PUBLIC -lz)
#these can also be done as #define statements, but they MUST appear before rmkit.h
#all these defines are necessary to build successfully
#they're here for convenience (you can never forget them) and also to ensure
#they appear early enough in the linking stage
target_compile_definitions(${PROJECT_NAME} PRIVATE
        #RMKIT_IMPLEMENTATION
        #STB_IMAGE_IMPLEMENTATION
        #STB_IMAGE_RESIZE_IMPLEMENTATION
        #STB_IMAGE_WRITE_IMPLEMENTATION
        #STB_TRUETYPE_IMPLEMENTATION
        #when building for native hardware, REMARKABLE must be defined
        $<$<STREQUAL:${TARGET_PLATFORM},rm>:REMARKABLE>
        #debug builds require an external font file. I've provided one for you in /include
        $<$<STREQUAL:${TARGET_PLATFORM},x86>:FONT_EMBED_H="font_embed.h">
        )
#see also: rm-toolchain.cmake

target_link_libraries(${PROJECT_NAME} PRIVATE opkg)

#add_library(rmkit ${RMKIT_FILE})
#set_target_properties(rmkit PROPERTIES LINKER_LANGUAGE CXX)
#target_precompile_headers(rmkit PRIVATE ${RMKIT_FILE})

#target_precompile_headers(${PROJECT_NAME} REUSE_FROM rmkit)
#target_link_libraries(${PROJECT_NAME} PRIVATE rmkit)


#below is an unfinished experiment in using rmkit-sim to do local emulation
#I'll finish it someday
#to enable local emulation, install resim (link here)
#if resim is not available in the PATH environment variable, you may specify it here
#set(RESIM_ALTERNATE_PATH "~/.local/bin/")

#find_program(RESIM_PATH resim
#        "$ENV{PATH}" "${RESIM_ALTERNATE_PATH}")

#if(NOT RESIM_PATH)
#    message(WARNING "Resim is not found in any search path. Local emulation disabled!")
#endif ()

#if(RESIM_PATH)
##    message(NOTICE "Resim found at ${RESIM_PATH}. Setting up target...")
#    add_custom_target(Simulate
#            COMMAND ${RESIM_PATH} ${CMAKE_BINARY_DIR}/${PROJECT_NAME}
#            VERBATIM)
#    add_dependencies(Simulate ${PROJECT_NAME})
##else()
##    message(NOTICE "Resim path not found, Simulate target disabled!")
#endif()


add_custom_target(docker_release
        COMMAND docker run -i -v "${CMAKE_SOURCE_DIR}:/mnt/project" --pull missing -w /mnt/project ghcr.io/toltec-dev/toolchain:latest sh docker_build_script.sh
        VERBATIM)

add_custom_target(run_app
        COMMAND cd .. && sh run_app.sh ${CMAKE_PROJECT_NAME}
        VERBATIM)
#cause run_app to trigger a docker build if it hasn't run yet
#Clion users may want to change this to (run_app ${CMAKE_PROJECT_NAME}) to not invoke docker
add_dependencies(run_app ${PROJECT_NAME})
