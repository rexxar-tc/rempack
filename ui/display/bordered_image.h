//
// Created by brant on 7/4/25.
//

#pragma once
#include "widgets.h"
namespace widgets {

    class BorderedPixmap: public widgets::RoundCornerWidget{
    public:
        BorderedPixmap(int x, int y, int w, int h, icons::Icon ico, RoundCornerStyle style);
        void on_reflow() override;
        void show() override;
        void hide() override;
        void mark_redraw() override;
        void before_render() override;
        void clearImage();
        void setAspectWidth(int imageX, int imageY);
        void setImage(icons::Icon icon);
        void setImage(ui::CachedIcon icon);
        void setImage(icons::Icon icon, int w, int h);
        void setImage(ui::CachedIcon icon, int w, int h);
        shared_ptr<widgets::RoundCornerWidget> dropShadow(int offsetX, int offsetY, RoundCornerStyle shadowStyle);
    private:
        shared_ptr<widgets::RoundCornerWidget> shadow;
        shared_ptr<ui::Pixmap> image;
    };

} // widgets
