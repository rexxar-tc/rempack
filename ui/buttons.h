//
// Created by brant on 7/3/25.
//

#pragma once
#include "widgets.h"
#include "data.h"

namespace widgets {

    class EventButton : public ui::Button{
    public:
        EventButton(int x, int y, int w, int h, string text = "") : Button(x, y, w, h, text){}

        PLS_DEFINE_SIGNAL(BUTTON_EVENT, void*);

        class BUTTON_EVENTS {
        public:
            BUTTON_EVENT clicked;
        };

        BUTTON_EVENTS events;
        void on_mouse_click(input::SynMotionEvent &ev) override;

        void disable();

        void enable();

        void render() override;
    protected:
        bool enabled = true;
    };

//basically a reimplementation of ui::Button with a clickable image instead of text
    class ImageButton : public EventButton {
    public:
        ImageButton(int x, int y, int w, int h, icons::Icon icon);


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
        void on_reflow()override;
    };




} // widgets
