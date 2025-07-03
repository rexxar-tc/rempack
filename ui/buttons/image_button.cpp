//
// Created by brant on 7/3/25.
//

#include "image_button.h"
namespace widgets{
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