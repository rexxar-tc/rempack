//
// Created by brant on 7/3/25.
//

#pragma once

#include "widgets.h"
#include "buttons/buttons.h"
#include "display/bordered_image.h"
#include "opkg.h"

namespace widgets {
    /*
    * _________________________________________________________________
    * | Package info                                 ________________ |
    * | Package name                                 | Splashscreen | |
    * | Package version                              |    image     | |
    * | Package etc                                  |   preview    | |
    * |                                              |              | |
    * | [Install(Upgrade)] [Uninstall]               ---------------- |
    * ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
    */
    class PackageInfoPanel: public RoundCornerWidget{
    public:
        PackageInfoPanel(int x, int y, int w, int h, RoundCornerStyle style, shared_ptr<ui::InnerScene> &scene);

        PLS_DEFINE_SIGNAL(PACKAGE_EVENT, void*);

        class PACKAGE_EVENTS {
        public:
            PACKAGE_EVENT install;
            PACKAGE_EVENT uninstall;
            PACKAGE_EVENT preview;
        };

        PACKAGE_EVENTS events;

        void display_package(const shared_ptr<package>& package);
        void on_reflow() override;
        void set_image(const shared_ptr<package>& package);
    private:
        int padding = 5;
        int controlHeight = 40;
        int controlWidth = 200;
        shared_ptr<ui::MultiText> _text;
        shared_ptr<EventButton> _installBtn, _removeBtn, _previewBtn;
        shared_ptr<BorderedPixmap> _image;

        void layout_buttons();

        void set_text(string text);
        void set_states(bool installed, bool canPreview = false);
        void layout_image();
    };

} // widgets
