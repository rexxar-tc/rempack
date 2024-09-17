add_library(opkg STATIC opkg.cpp)

target_include_directories(opkg PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

find_package(ZLIB)

target_link_options(opkg PRIVATE -lz)

target_link_libraries(opkg PRIVATE ZLIB::ZLIB)