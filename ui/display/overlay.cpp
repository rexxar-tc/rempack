//
// Created by brant on 7/2/25.
//

#include "overlay.h"

namespace widgets {

    void Overlay::show() {
        if (!this->scene)
            build_dialog();
        visible = true;
        //mark_redraw();
        this->scene->pinned = pinned;
        ui::MainLoop::show_overlay(this->scene, stack);
    }

    void Overlay::hide() {
        visible = false;
        ui::MainLoop::hide_overlay(this->scene);
    }

    void Overlay::render() {
        border_widget->set_coords(x, y, w, h);
        border_widget->render_inside_fill();
//for(const auto &child : scene->widgets)
//    draw_recurse(child);
    }

    void Overlay::draw_recurse(const shared_ptr<ui::Widget> &w) {
        fb->draw_rect(w->x, w->y, w->w, w->h, BLACK, false);
        if (w->children.size()) {
            for (const auto &child: w->children)
                draw_recurse(child);
        }
    }

    void Overlay::mark_redraw() {
        for (const auto &c: children)
            c->mark_redraw();
        this->titleWidget->mark_redraw();
        this->contentWidget->mark_redraw();
        this->dirty = 1;
    }

    void Overlay::on_button_selected(std::string s) {
        ui::Dialog::on_button_selected(s);
        this->hide();
    }
}