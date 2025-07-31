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

        bool is_enabled();

        void render() override;

        void render_border() override;

        void mark_redraw() override;

        void on_reflow() override;

        void show() override;

        void hide() override;

        void enableBorder(bool enable);

        void undraw() override;

        RoundCornerStyle border_style;
        shared_ptr<RoundCornerWidget> border;
    protected:
        bool enabled = true;
        bool borderEnabled = true;

        virtual void debugRender();
    };
}