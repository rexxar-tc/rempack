add_library(opkg STATIC opkg.cpp)
target_include_directories(opkg PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})