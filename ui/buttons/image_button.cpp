//
// Created by brant on 7/3/25.
//

#include "image_button.h"

namespace widgets{
    void ImageButton::render() {
        //pixmap->render();
        EventButton::render();
        //fb->waveform_mode = WAVEFORM_MODE_GC16;
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
        border->set_coords(x, y, w, h);
        border->mark_redraw();
        ui::Rect::on_reflow();
    }

    void ImageButton::on_mouse_click(input::SynMotionEvent &ev) {
        ev.stop_propagation();
        if (!enabled)
            return;
        EventButton::on_mouse_click(ev);
    }

    ImageButton::ImageButton(int x, int y, int w, int h, icons::Icon icon, RoundCornerStyle style) : EventButton(x, y, w, h, "", style){
        pixmap = make_shared<ui::Pixmap>(x, y, w, h, icon);
        pixmap->alpha = 0xFF;
        children.push_back(pixmap);
    }

    ImageButton::ImageButton(int x, int y, int w, int h, const string &text, RoundCornerStyle style) : EventButton(x, y, w, h, text, style){
        pixmap = make_shared<ui::Pixmap>(x, y, w, h, icons::Icon {});
        pixmap->alpha = 0xFF;
        children.push_back(pixmap);
    }

}