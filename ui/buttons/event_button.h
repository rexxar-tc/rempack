//
// Created by brant on 7/3/25.
//

#pragma once

#include "widgets.h"

namespace widgets {
    class EventButton : public ui::Button {
    public:
        EventButton(int x, int y, int w, int h, string text = "");
        EventButton(int x, int y, int w, int h, string text, RoundCornerStyle style);
        EventButton(int x, int y, int w, int h, RoundCornerStyle style);

        PLS_DEFINE_SIGNAL(BUTTON_EVENT,
        void*);

        class BUTTON_EVENTS {
        public:
            BUTTON_EVENT clicked;
        };

        BUTTON_EVENTS events;

        void on_mouse_click(input::SynMotionEvent &ev) override;

        void disable();

        void enable();

        void render() override;

        void render_border() override;

        RoundCornerStyle border_style;
        shared_ptr<RoundCornerWidget> border;
    protected:
        bool enabled = true;
        virtual void debugRender();
    };
}