//
// Created by brant on 7/3/25.
//

#pragma once

#include "widgets.h"
#include "buttons/event_button.h"
#include "opkg.h"

namespace widgets {
    /*
    * _____________________________________________________________________________________
    * | Package info                                                                      |
    * | Package name                                                                      |
    * | Package version                                                                   |
    * | Package etc                                                                       |
    * |                                                                                   |
    * | [Install(Upgrade)] [Uninstall] /\*[Download] [Preview]     [Pending: [+0/-0]] *\/ |
    * ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
    */
    class PackageInfoPanel: public RoundCornerWidget{
    public:
        int padding = 5;
        int controlHeight = 40;
        int controlWidth = 200;
        PackageInfoPanel(int x, int y, int w, int h, RoundCornerStyle style, shared_ptr<ui::InnerScene> &scene) : RoundCornerWidget(x,y,w,h,style){
            _text = make_shared<ui::MultiText>(x,y,w,h,"");
            _text->set_coords(x+padding,y+padding,w-(2*padding),h-(2*padding) - controlHeight);
            children.push_back(_text);
            _installBtn = make_shared<EventButton>(x,y,200, controlHeight,"Install");
            _removeBtn = make_shared<EventButton>(x,y,200, controlHeight,"Uninstall");
            _downloadBtn = make_shared<EventButton>(x,y,200, controlHeight,"Download");
            _previewBtn = make_shared<EventButton>(x,y,200, controlHeight,"Preview");
            _actionCounter = make_shared<EventButton>(x,y,200, controlHeight, "Pending: [+0/-0]");
            children.push_back(_installBtn);
            scene->add(_installBtn);
            children.push_back(_removeBtn);
            scene->add(_removeBtn);
            //children.push_back(_downloadBtn);
            //children.push_back(_previewBtn);
            //children.push_back(_actionCounter);
            _installBtn->events.clicked += [this](void*){events.install();};
            _removeBtn->events.clicked += [this](void*){events.uninstall();};
            _downloadBtn->events.clicked += [this](void*){events.download();};
            _previewBtn->events.clicked += [this](void*){events.preview();};
            _actionCounter->events.clicked += [this](void*){events.enact();};
            layout_buttons();
        }

        PLS_DEFINE_SIGNAL(PACKAGE_EVENT, void*);

        class PACKAGE_EVENTS {
        public:
            PACKAGE_EVENT install;
            PACKAGE_EVENT uninstall;
            PACKAGE_EVENT download;
            PACKAGE_EVENT preview;
            PACKAGE_EVENT enact;
        };

        PACKAGE_EVENTS events;

        void on_reflow() override;
        void set_text(string text);
        void set_image(const shared_ptr<package>& package);
        void set_states(bool installed, bool canPreview = false, bool showDownload = false);
        void set_actions(int add, int remove);
    private:
        shared_ptr<ui::MultiText> _text;
        shared_ptr<EventButton> _installBtn, _removeBtn, _downloadBtn, _previewBtn, _actionCounter;
        shared_ptr<ui::Pixmap> _pixmap;

        void layout_buttons();
        void layout_image();
    };

} // widgets
