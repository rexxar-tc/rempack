//
// Created by brant on 7/4/25.
//

#include "bordered_image.h"

namespace widgets {
    BorderedPixmap::BorderedPixmap(int x, int y, int w, int h, icons::Icon ico, RoundCornerStyle style) : Pixmap(x, y, w, h, ico){
        border = make_shared<RoundCornerWidget>(x,y,w,h,style);
        children.push_back(border);
    }

    void BorderedPixmap::on_reflow() {
        border->set_coords(x, y, w, h);
        border->mark_redraw();
        Rect::on_reflow();
    }

    void BorderedPixmap::show() {
        border->show();
        Widget::show();
    }

    void BorderedPixmap::hide() {
        border->hide();
        Widget::hide();
    }
} // widgets