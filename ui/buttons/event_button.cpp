//
// Created by brant on 7/3/25.
//
#include "event_button.h"

namespace widgets {
    EventButton::EventButton(int x, int y, int w, int h, string text) : Button(x, y, w, h, text) {
        x_padding = 0;
        y_padding = 0;
    }

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
#ifdef WIDGET_DEBUG
        this->debugRender();
#endif
    }

    void EventButton::debugRender() {
        fb->draw_rect(x,y,w,h, toRColor(128,0,128),false);
        auto t = textWidget;
        fb->draw_rect(t->x+2, t->y+2, t->w-4,t->h-4, toRColor(0,255,128),false);
    }
}