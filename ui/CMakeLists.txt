add_library(widgets
        widgets.cpp
        input/keyboard/keyboard.cpp
        display/list_box.cpp
        display/overlay.cpp
        buttons/event_button.cpp
        buttons/image_button.cpp
        input/rounded_text_input.cpp
        input/labeled_range.cpp
        rempack/dialog/terminal_dialog.cpp
        rempack/dialog/install_dialog.cpp
        rempack/dialog/uninstall_dialog.cpp
        rempack/overlay/menu_overlay.cpp
        rempack/overlay/filter_overlay.cpp
        rempack/search_box.cpp
        rempack/filter_button.cpp
        rempack/package_info_panel.cpp
        rempack/config_button.cpp
)

include_directories(../opkg)
target_link_libraries(widgets PRIVATE opkg)
target_include_directories(widgets PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})