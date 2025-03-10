//
// Created by brant on 2/18/24.
//

#pragma once

#include "rempack_widgets.h"
namespace widgets {
    class Overlay : public ui::Dialog {
    private:
        shared_ptr<widgets::RoundCornerWidget> border_widget;
    public:
        bool pinned = false;
        bool stack = true;

        Overlay(int x, int y, int w, int h) : ui::Dialog(x, y, w, h) {
            border_widget = make_shared<widgets::RoundCornerWidget>(x, y, w, h, widgets::RoundCornerStyle());
            children.push_back(border_widget);
        }

        void show() override {
            if(!this->scene)
                build_dialog();
            visible = true;
            //mark_redraw();
            this->scene->pinned = pinned;
            ui::MainLoop::show_overlay(this->scene, stack);
        }

        void hide() override {
            visible = false;
            ui::MainLoop::hide_overlay(this->scene);
        }

        void render() override {
            border_widget->set_coords(x, y, w, h);
            border_widget->render_inside_fill();
            //for(const auto &child : scene->widgets)
            //    draw_recurse(child);
        }

        void draw_recurse(const shared_ptr<ui::Widget> &w){
            fb->draw_rect(w->x, w->y, w->w, w->h, BLACK, false);
            if(w->children.size()){
                for(const auto &child : w->children)
                    draw_recurse(child);
            }
        }

        void mark_redraw() override {
            for (const auto &c: children)
                c->mark_redraw();
            this->titleWidget->mark_redraw();
            this->contentWidget->mark_redraw();
            this->dirty = 1;
        }

        void on_button_selected(std::string s) override{
            ui::Dialog::on_button_selected(s);
            this->hide();
        }
    };


}