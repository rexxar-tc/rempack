//
// Created by brant on 7/2/25.
//

#include "include/widgets.h"

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

    void ImageButton::render() {
        EventButton::render();
        //fb->waveform_mode = WAVEFORM_MODE_GC16;
        pixmap->render();
    }

    void ImageButton::on_reflow() {
        //pixmap->undraw();
        auto dw = min(w, h);
        auto dx = x + (w / 2) - (dw / 2);
        pixmap->set_coords(dx, y, dw, dw);
        //pixmap->icon.width  = dw;
        //pixmap->icon.height = dw;
        //util::resize_image(pixmap->icon.image, dw, dw, 20);
        pixmap->on_reflow();
        pixmap->mark_redraw();
    }

    void ImageButton::on_mouse_click(input::SynMotionEvent &ev) {
        ev.stop_propagation();
        if (!enabled)
            return;
        EventButton::on_mouse_click(ev);
    }

    void RoundImageButton::on_reflow() {
        border->set_coords(x, y, w, h);
        border->mark_redraw();
        ImageButton::on_reflow();
    }
}
