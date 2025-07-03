//
// Created by brant on 7/3/25.
//
#include "event_button.h"

namespace widgets {
    void EventButton::on_mouse_click(input::SynMotionEvent &ev) {
        ev.stop_propagation();
        if (!enabled)
            return;
        events.clicked();
        mark_redraw();
    }

    void EventButton::disable() {
        enabled = false;
        mark_redraw();
    }

    void EventButton::enable() {
        enabled = true;
        mark_redraw();
    }

    void EventButton::render() {
        ui::Button::render();
        if (!enabled) {
            fb->draw_rect(x, y, w, h, color::GRAY_12, true);
        }
        //fb->draw_rect(x,y,w,h,BLACK,false);
    }
}