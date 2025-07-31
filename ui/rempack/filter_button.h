//
// Created by brant on 7/3/25.
//

#pragma once
#include "widgets.h"
#include "overlay/filter_options.h"
#include "buttons/image_button.h"

namespace widgets{
    class FilterButton:public ImageButton{
    public:
        shared_ptr<FilterOptions> options;
        FilterButton(int x, int y, int w, int h, shared_ptr<FilterOptions> defaultOptions, RoundCornerStyle style = RoundCornerStyle()) : ImageButton(x, y, w, h, ICON(assets::png_filter_png), style) {
            options = defaultOptions;
        }
        PLS_DEFINE_SIGNAL(FILTER_EVENT, FilterOptions);

        class FILTER_EVENTS: public BUTTON_EVENTS {
        public:
            FILTER_EVENT updated;
        };

        FILTER_EVENTS events;

        void on_overlay_hidden(ui::InnerScene::DialogVisible v);
        void on_mouse_click(input::SynMotionEvent &ev) override;
    };
}