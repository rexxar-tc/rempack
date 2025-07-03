//
// Created by brant on 7/3/25.
//

#pragma once

#include "buttons/buttons.h"
#include "rempack/overlay/menu_data.h"

namespace widgets{
    class ConfigButton : public RoundImageButton {
    public:
        RoundCornerStyle style;
        MenuData *data;
        ConfigButton(int x, int y, int w, int h, MenuData *data, RoundCornerStyle style = RoundCornerStyle()) : RoundImageButton(x, y, w, h, ICON(assets::png_menu_png), style) {
            this->data = data;
        }
        void on_overlay_hidden(ui::InnerScene::DialogVisible v);
        void on_mouse_click(input::SynMotionEvent &ev) override;
    };

}