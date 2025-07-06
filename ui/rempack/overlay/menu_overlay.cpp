//
// Created by brant on 7/3/25.
//

#include "menu_overlay.h"
#include "rempack/dialog/terminal_dialog.h"
#include "buttons/event_button.h"
#include "opkg.h"

namespace widgets {
    MenuOverlay::MenuOverlay(int x, int y, int w, int h, MenuData *currentData) : RoundCornerWidget(x,y,w,h,RoundCornerStyle()) {
        data = currentData;
        scene = make_overlay();
    }

    void MenuOverlay::show() {
        //this->scene->pinned = true;
        ui::MainLoop::show_overlay(this->scene, true);
        ui::MainLoop::refresh();
    }

    void MenuOverlay::hide() {
        ui::MainLoop::hide_overlay(this->scene);
        ui::MainLoop::refresh();
    }

    void MenuOverlay::debugRender() {
        for(const auto &c : scene->widgets){
            if(c.get() != this)
                fb->draw_rect(c->x, c->y, c->w, c->h, toRColor(0,0,255), false);
        }
        //RoundCornerWidget::debugRender();
    }

    void MenuOverlay::render() {
        render_inside_fill();
        RoundCornerWidget::render();
    }

    void MenuOverlay::mark_redraw() {
        for (const auto &c: children)
            c->mark_redraw();
        this->dirty = 1;
    }

    void MenuOverlay::upate_event() {
        events.updated(data);
        mark_redraw();
    }

    void MenuOverlay::refresh_event(void *) {
        auto td = new TerminalDialog(500, 500, 800, 1100, "opkg update");
        td->set_callback([this]() { this->hide(); });
        td->show();
        ui::TaskQueue::add_task([=]() {
            auto ret = opkg::UpdateRepos([=](const string &s) { td->stdout_callback(s); });
            if (ret == 0)
                td->stdout_callback("Done.");
            else
                td->stdout_callback("Error!");
            std::cout << "opkg update returned with exit code " << ret << std::endl;
        });
    }

    ui::Scene MenuOverlay::make_overlay() {
        int padding = 20;
        auto s = ui::make_scene();
        s->add(this);
        mark_redraw();
        auto v = ui::VerticalLayout(x, y + padding, 500, 800 - (padding * 4), s);
        auto dw = 500 - padding - padding;
        auto dh = 800 - padding - padding;
        auto dx = v.x + padding;
        auto dy = v.y + padding;
        auto updateBtn = new EventButton(padding, padding, dw, utils::line_height(), "Refresh Repositories");
        updateBtn->events.clicked += PLS_DELEGATE(refresh_event);
        v.pack_start(updateBtn);
        auto exitBtn = new EventButton(padding, 0 - padding - utils::line_height(), dw, utils::line_height(), "Quit");
        exitBtn->events.clicked += PLS_LAMBDA(auto &ev) { exit(0); };
        v.pack_end(exitBtn);
        return s;
    }
} // widgets