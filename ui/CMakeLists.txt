add_library(widgets
        widgets.cpp
        buttons.cpp
        keyboard.cpp
        list_box.cpp
        overlay.cpp
        rempack_widgets.cpp
)

include_directories(../opkg)
target_link_libraries(widgets PRIVATE opkg)
target_include_directories(widgets PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})