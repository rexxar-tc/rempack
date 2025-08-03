//
// Created by brant on 8/1/25.
//

#pragma once

#include "widgets.h"
#include "ScreenCatcher.h"
#include <stack>

namespace fs = std::filesystem;
namespace debugging{
    std::vector<shared_ptr<ui::Widget>> walk_scene(const ui::InnerScene& scene){
        std::vector<shared_ptr<ui::Widget>> res;
        std::stack<shared_ptr<ui::Widget>> stack;
        for(const auto &w : scene.widgets){
            stack.push(w);

            while(!stack.empty()){
                auto &c = stack.top();
                stack.pop();
                res.push_back(c);

                for(const auto &cc : c->children){
                    stack.push(cc);
                }
            }
        }

        return res;
    }

    namespace {
        /*
         * Three levels:
         * Full screen
         * Full widget
         * Individual widget
         *
         */
        void render_layer(framebuffer::FB &fb, framebuffer::FB &fb2, shared_ptr<ui::Widget> &widget, const fs::path &basePath, int wakeSig) {

            if (!widget->visible)
                return;

            framebuffer::VirtualFB fb3(fb.width, fb.height);
            fb3.clear_screen();
            auto ofb = widget->fb;
            widget->fb = &fb;
            widget->render();
            widget->render_border();
            widget->fb = &fb2;
            widget->render();
            widget->render_border();
            widget->fb = &fb3;
            widget->render();
            widget->render_border();
            widget->fb = ofb;
            auto g = widgets::toRColor(0,255,0);
            auto r = widgets::toRColor(255,0,0);
            fb2.draw_rect(widget->x, widget->y, widget->w, widget->h, g, false); // NOLINT
            for(int i = -1; i < 3; i++ ) {
                fb3.draw_rect(widget->_x - i, widget->_y - i, widget->_w + (2 * i), widget->_h + (2 * i), r, false); // NOLINT
            }
            fb3.draw_rect(widget->x, widget->y, widget->w, widget->h, g, false); // NOLINT

            if (fb3.dirty) {
                //update
                auto oPath = basePath;
                oPath += "_3";
                oPath.replace_extension(".png");
                ScreenCatcher::WriteScreen(oPath, fb3.fbmem, fb3.width, fb3.height, wakeSig);
            }
            if(!widget->children.empty()){
                int j = 0;
                for (auto &cc: widget->children) {
                    render_layer(fb, fb3, cc, basePath / to_string(j++), wakeSig);
                }
            }
            if (fb2.dirty) {
                //update
                auto oPath = basePath;
                oPath += "_2";
                oPath.replace_extension(".png");
                ScreenCatcher::WriteScreen(oPath, fb2.fbmem, fb2.width, fb2.height, wakeSig);
            }
        }
    }

    //1404x1872 - 157x209mm -- 226dpi
    void render_debug_layers(const shared_ptr<ui::InnerScene>& scene, const string& basePath, int wakeSig) {

        auto fb = framebuffer::VirtualFB(1404, 1872);
        auto fb2 = framebuffer::VirtualFB(1404, 1872);
        fb.clear_screen();
        fb2.clear_screen();
        fs::path path = basePath;
        int count = 0;
        for (auto &w: scene->widgets) {
            render_layer(fb, fb2, w, path / to_string(count++), wakeSig);
            fs::path lpath = path;
            lpath += "_1";
            lpath.replace_extension(".png");
            if(fb2.dirty)
            {
                ScreenCatcher::WriteScreen(lpath, fb2.fbmem, fb2.width, fb2.height, wakeSig);
                fb2.clear_screen();
            }
        }

            auto oPath = path;
            oPath.replace_extension(".png");
            ScreenCatcher::WriteScreen(oPath, fb.fbmem, fb.width, fb.height, wakeSig);

    }
}