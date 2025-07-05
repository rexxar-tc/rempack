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
    private:
        shared_ptr<RoundCornerWidget> border;
    };

} // widgets
