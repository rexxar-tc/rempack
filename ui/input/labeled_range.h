//
// Created by brant on 7/3/25.
//

#pragma once

#include "widgets.h"
#include "input.h"

namespace widgets{

    class LabeledRangeInput : public ui::Widget {
    public:
        enum LabelPosition {
            LEFT, TOP
        };

        LabeledRangeInput(int x, int y, int w, int h, string text = "", LabelPosition pos = LEFT, int padding = 5)
                : ui::Widget(x, y, w, h) {
            if (!text.empty()) {
                label = make_shared<ui::Text>(x, y, w, (h / 2) - padding, text);
                children.push_back(label);
                y += h / 2;
                h = h / 2 - padding;
            }
            range = make_shared<ui::RangeInput>(x, y, w, h);

            children.push_back(range);
            w = range->x + range->w - x;
            h = range->y + range->h - y;
        }

        void mark_redraw() override;


        shared_ptr<ui::RangeInput> range;
    private:
        shared_ptr<ui::Text> label = nullptr;
    };

}