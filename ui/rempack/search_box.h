//
// Created by brant on 7/3/25.
//

#pragma once
#include "input/input.h"

namespace widgets {
    class SearchBox : public RoundedTextInput {
    public:
        PLS_DEFINE_SIGNAL(TEXTBOX_EVENT, string);

        class TEXTBOX_EVENTS: public TEXTINPUT_EVENTS {
        public:
            TEXTBOX_EVENT updated;
            TEXTBOX_EVENT open;
        };

        TEXTBOX_EVENTS events;
        SearchBox(int x, int y, int w, int h, RoundCornerStyle style, string text = "");
        void on_reflow() override;
        void on_mouse_click(input::SynMotionEvent &ev) override;
    private:
        widgets::Keyboard *_keyboard;
        shared_ptr<ui::Pixmap> pixmap;
        void onChange(KeyboardEvent ev);
        void onDone(KeyboardEvent ev);
    };

} // widgets
