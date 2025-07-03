//
// Created by brant on 7/3/25.
//

#pragma once

#include "widgets.h"
#include "event_button.h"

namespace widgets{
    //basically a reimplementation of ui::Button with a clickable image instead of text
    class ImageButton : public EventButton {
    public:
        ImageButton(int x, int y, int w, int h, icons::Icon icon) : EventButton(x, y, w, h, "  "){
            pixmap = make_shared<ui::Pixmap>(x, y, w, h, icon);
            pixmap->alpha = WHITE;
            children.push_back(pixmap);
        };


        void render() override;

        void on_reflow() override;

        void on_mouse_click(input::SynMotionEvent &ev) override;


    private:
        shared_ptr<ui::Pixmap> pixmap;
    };

    class RoundImageButton : public ImageButton{
    public:

        shared_ptr<RoundCornerWidget> border;
        RoundImageButton(int x, int y, int w, int h, icons::Icon icon, RoundCornerStyle style): ImageButton(x,y,w,h,icon){
            border = make_shared<RoundCornerWidget>(x,y,w,h,style);
            children.push_back(border);
        }
        void on_reflow()override;
    };

}