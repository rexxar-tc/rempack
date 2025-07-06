add_library(widgets STATIC
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
        display/bordered_image.cpp
        display/bordered_image.h
        rempack/dialog/conflict_dialog.cpp
        rempack/dialog/conflict_dialog.h
        dispatcher.cpp
        dispatcher.h
)

include_directories(../opkg)
target_link_libraries(widgets PUBLIC opkg rmkit_impl stb_impl)