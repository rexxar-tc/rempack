//
// Created by brant on 7/4/25.
//

#pragma once
#include "widgets.h"
namespace widgets {

    class BorderedPixmap: public ui::Pixmap {
    public:
        BorderedPixmap(int x, int y, int w, int h, icons::Icon ico, RoundCornerStyle style);
        void on_reflow() override;
        void show() override;
        void hide() override;
    private:
        shared_ptr<RoundCornerWidget> border;
    };

} // widgets
