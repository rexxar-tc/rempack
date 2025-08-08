//
// Created by brant on 7/4/25.
//

#include "bordered_image.h"

namespace widgets {
    BorderedPixmap::BorderedPixmap(int x, int y, int w, int h, icons::Icon ico, RoundCornerStyle style) : ui::Widget(x,y,w,h), DebuggableWidget(x,y,w,h), widgets::RoundCornerWidget(x, y, w, h, style){
        image = make_shared<ui::Pixmap>(x,y,w,h,ico);
        children.push_back(image);
    }

    void BorderedPixmap::center_image(){
        int dx = (this->w / 2) - (image->w/2) + this->x;
        int dy = (this->h / 2) - (image->h/2) + this->y;
        image->x = dx;
        image->y = dy;
    }

    void BorderedPixmap::mark_redraw() {
        image->mark_redraw();
        Widget::mark_redraw();
    }

    void BorderedPixmap::on_reflow() {
        center_image();
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

    void BorderedPixmap::setImage(const ui::CachedIcon& icon) {
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

    void BorderedPixmap::setImage(icons::Icon icon, int i_w, int i_h) {
        int dx = (this->w / 2) - (i_w/2) + x;
        int dy = (this->h / 2) - (i_h/2) + y;
        image->set_coords(dx, dy, i_w, i_h);
        image->icon = ui::CachedIcon(icon.data, icon.len, icon.name, i_w, i_h);
        image->undraw();
        image->show();
        mark_redraw();
    }

    int BorderedPixmap::getWidthForAspect(int i_w, int i_h) {
        auto aspect = (float)i_w / i_h;
        auto dw = (int)((float)this->h * aspect);
        return dw;
    }

    void BorderedPixmap::setAspectWidth(int imageX, int imageY) {
        int dw = getWidthForAspect(imageX, imageY);
        if(dw == this->w)
            return;

        undraw();
        set_coords(this->x, this->y, dw, this->h);
        mark_redraw();
    }

    void BorderedPixmap::debugRender() {
        DebuggableWidget::debugRender();
        fb->draw_rect(image->x, image->y, image->w, image->h, toRColor(64,128,255), false);
    }
} // widgets