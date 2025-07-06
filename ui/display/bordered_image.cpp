//
// Created by brant on 7/4/25.
//

#include "bordered_image.h"

namespace widgets {
    BorderedPixmap::BorderedPixmap(int x, int y, int w, int h, icons::Icon ico, RoundCornerStyle style) : widgets::RoundCornerWidget(x, y, w, h, style){
        image = make_shared<ui::Pixmap>(x,y,w,h,ico);
        children.push_back(image);
    }

    void BorderedPixmap::mark_redraw() {
        image->mark_redraw();
        Widget::mark_redraw();
    }

    void BorderedPixmap::on_reflow() {
        //image->set_coords(x, y, w, h);
        //image->mark_redraw();
        Rect::on_reflow();
    }

    void BorderedPixmap::show() {
        image->show();
        Widget::show();
    }

    void BorderedPixmap::hide() {
        image->hide();
        Widget::hide();
    }

    void BorderedPixmap::clearImage() {
        image->hide();
        mark_redraw();
    }

    void BorderedPixmap::setImage(ui::CachedIcon icon) {
        image->set_coords(x, y, w, h);
        image->icon = icon;
        image->show();
        //fb->update_mode = UPDATE_MODE_FULL;
        mark_redraw();
    }

    void BorderedPixmap::setImage(icons::Icon icon) {
        image->set_coords(x, y, w, h);
        image->icon = ui::CachedIcon(icon.data, icon.len, icon.name);
        image->show();
        //fb->update_mode = UPDATE_MODE_FULL;
        mark_redraw();
    }

    void BorderedPixmap::setImage(ui::CachedIcon icon, int w, int h) {
        int dx = (this->w / 2) - (w/2) + x;
        int dy = (this->h / 2) - (h/2) + y;
        image->set_coords(dx, dy, w, h);
        image->icon = ui::CachedIcon(icon.data, icon.len, icon.name, w, h);
        image->undraw();
        image->show();
        //fb->update_mode = UPDATE_MODE_FULL;
        mark_redraw();
    }

    void BorderedPixmap::setImage(icons::Icon icon, int w, int h) {
        int dx = (this->w / 2) - (w/2) + x;
        int dy = (this->h / 2) - (h/2) + y;
        image->set_coords(dx, dy, w, h);
        image->icon = ui::CachedIcon(icon.data, icon.len, icon.name, w, h);
        image->undraw();
        image->show();
        //fb->update_mode = UPDATE_MODE_FULL;
        mark_redraw();
    }
} // widgets