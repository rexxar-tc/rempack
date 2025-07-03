find_package(ZLIB)
find_package(LibArchive)

add_library(opkg
        opkg.cpp
        parser.cpp
        formatting.cpp
        cache.cpp
)
#target_link_libraries(opkg PRIVATE LibArchive::LibArchive)
target_link_libraries(opkg PRIVATE ZLIB::ZLIB)
target_include_directories(opkg PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})