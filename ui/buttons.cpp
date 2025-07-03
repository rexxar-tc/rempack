//
// Created by brant on 7/3/25.
//

#include "buttons.h"

namespace widgets {
    void RoundImageButton::on_reflow() {
        border->set_coords(x,y,w,h);
        border->mark_redraw();
        ImageButton::on_reflow();
    }

    RoundImageButton::RoundImageButton(int x, int y, int w, int h, icons::Icon icon, RoundCornerStyle style) : ImageButton(x,y,w,h,icon){
        border = make_shared<RoundCornerWidget>(x,y,w,h,style);
        children.push_back(border);
    }

    void EventButton::on_mouse_click(input::SynMotionEvent &ev) {
        ev.stop_propagation();
        if(!enabled)
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
        if(!enabled) {
            fb->draw_rect(x, y, w, h, color::GRAY_12, true);
        }
        //fb->draw_rect(x,y,w,h,BLACK,false);
    }

    ImageButton::ImageButton(int x, int y, int w, int h, icons::Icon icon) : EventButton(x, y, w, h, "") {
        pixmap = make_shared<ui::Pixmap>(x, y, w, h, icon);
        pixmap->alpha = WHITE;
        children.push_back(pixmap);
    }

    void ImageButton::render() {
        EventButton::render();
        //fb->waveform_mode = WAVEFORM_MODE_GC16;
        pixmap->render();
    }

    void ImageButton::on_reflow() {
        //pixmap->undraw();
        auto dw = min(w,h);
        auto dx = x+(w/2) - (dw/2);
        pixmap->set_coords(dx, y, dw, dw);
        //pixmap->icon.width  = dw;
        //pixmap->icon.height = dw;
        //util::resize_image(pixmap->icon.image, dw, dw, 20);
        pixmap->on_reflow();
        pixmap->mark_redraw();
    }

    void ImageButton::on_mouse_click(input::SynMotionEvent &ev) {
        ev.stop_propagation();
        if(!enabled)
            return;
        EventButton::on_mouse_click(ev);
    }
} // widgets