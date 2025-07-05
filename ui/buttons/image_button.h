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
        ImageButton(int x, int y, int w, int h, icons::Icon icon);
        ImageButton(int x, int y, int w, int h, const string& text);

        void render() override;

        void on_reflow() override;

        void on_mouse_click(input::SynMotionEvent &ev) override;


    private:
        shared_ptr<ui::Pixmap> pixmap;
    };

    class RoundImageButton : public ImageButton{
    public:

        shared_ptr<RoundCornerWidget> border;
        RoundImageButton(int x, int y, int w, int h, icons::Icon icon, RoundCornerStyle style);
        RoundImageButton(int x, int y, int w, int h, const string &text, RoundCornerStyle style);
        void on_reflow()override;
    };

}