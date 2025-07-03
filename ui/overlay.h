//
// Created by brant on 2/18/24.
//

#pragma once

#include "widgets.h"
namespace widgets {
    class Overlay : public ui::Dialog {
    private:
        shared_ptr<widgets::RoundCornerWidget> border_widget;
    public:
        bool pinned = false;
        bool stack = true;

        Overlay(int x, int y, int w, int h);

        void show() override;

        void hide() override;

        void render() override;

        void draw_recurse(const shared_ptr<ui::Widget> &w);

        void mark_redraw() override;

        void on_button_selected(std::string s) override;
    };


}