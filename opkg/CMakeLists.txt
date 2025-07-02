
find_package(ZLIB)

add_library(opkg
        opkg.cpp
        parser.cpp
        formatting.cpp
        cache.cpp
)

target_link_libraries(opkg PRIVATE ZLIB::ZLIB)
target_include_directories(opkg PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})