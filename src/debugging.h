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
        //std::mutex mutex_;
        stack<shared_ptr<framebuffer::FB>> bufferPool;

        int buffer_count = 0;

        void release_buffer(const shared_ptr<framebuffer::FB>& fb){
            //std::lock_guard<std::mutex> lock(mutex_);
            fb->dirty = 0;
            memset(fb->fbmem, 0xff, fb->byte_size);
            bufferPool.push(fb);
        }

        shared_ptr<framebuffer::FB> get_buffer(int x, int y){
            //std::lock_guard<std::mutex> lock(mutex_);
            if(bufferPool.empty()) {
                std::cout << "allocations: " << buffer_count << std::endl;
                buffer_count++;
                return make_shared<framebuffer::VirtualFB>(x, y);
            }
            auto r = bufferPool.top();
            bufferPool.pop();
            return r;
        }

        class PooledBuffer{
        private:
            shared_ptr<framebuffer::FB> fb = nullptr;
        public:
            PooledBuffer(int x, int y){
                fb = get_buffer(x, y);
            }
            ~PooledBuffer(){
                release_buffer(fb);
            }

            shared_ptr<framebuffer::FB> operator->(){return fb;}
            shared_ptr<framebuffer::FB>& operator*(){return fb;}
            PooledBuffer(const PooledBuffer&) = delete;
            PooledBuffer& operator=(const PooledBuffer&) = delete;
            PooledBuffer(PooledBuffer&& other) noexcept : fb(other.fb) {
                other.fb = nullptr;
            }
            PooledBuffer& operator=(PooledBuffer&& other) noexcept {
                if (this != &other) {
                    if (fb) release_buffer(fb);
                    fb = other.fb;
                    other.fb = nullptr;
                }
                return *this;
            }
        };

        void render_layer(shared_ptr<framebuffer::FB> &globalFb, shared_ptr<framebuffer::FB> &parentFb, shared_ptr<ui::Widget> &widget, const fs::path &basePath, int wakeSig) {

            if (!widget->visible)
                return;

            //auto localFb = get_buffer(globalFb->width, globalFb->height);
            PooledBuffer localFb(globalFb->width, globalFb->height);
            auto ofb = widget->fb;
            widget->fb = globalFb.get();
            widget->render();
            widget->render_border();
            widget->fb = parentFb.get();
            widget->render();
            widget->render_border();
            widget->fb = (*localFb).get();
            widget->render();
            widget->render_border();
            widget->fb = ofb;
            auto g = widgets::toRColor(0,255,0);
            auto r = widgets::toRColor(255,0,0);
            parentFb->draw_rect(widget->x, widget->y, widget->w, widget->h, g, false); // NOLINT
            for(int i = -1; i < 3; i++ ) {
                localFb->draw_rect(widget->_x - i, widget->_y - i, widget->_w + (2 * i), widget->_h + (2 * i), r, false); // NOLINT
            }
            localFb->draw_rect(widget->x, widget->y, widget->w, widget->h, g, false); // NOLINT

            if (localFb->dirty) {
                //update
                auto oPath = basePath;
                //oPath += "_3";
                oPath.replace_extension(".png");
                ScreenCatcher::WriteScreen(oPath, localFb->fbmem, localFb->width, localFb->height, wakeSig);
            }
            if(!widget->children.empty()){
                int j = 0;
                for (auto &cc: widget->children) {
                    render_layer(globalFb, *localFb, cc, basePath / to_string(j++), wakeSig);
                }
            }
            //if (parentFb->dirty) {
            //    //update
            //    auto oPath = basePath;
            //    oPath += "_2";
            //    oPath.replace_extension(".png");
            //    ScreenCatcher::WriteScreen(oPath, parentFb->fbmem, parentFb->width, parentFb->height, wakeSig);
            //}
        }
    }

    //1404x1872 - 157x209mm -- 226dpi
    void render_debug_layers(const shared_ptr<ui::InnerScene>& scene, int x, int y, const string& basePath, int wakeSig) {

        auto fb = PooledBuffer(x, y);
        auto fb2 = PooledBuffer(x, y);
        fs::path path = basePath;
        path = path / path.filename();
        int count = 0;
        for (auto &w: scene->widgets) {
            render_layer(*fb, *fb2, w, path / to_string(count), wakeSig);
            fs::path lpath = path;
            lpath += to_string(count);
            lpath.replace_extension(".png");
            if (fb2->dirty) {
                ScreenCatcher::WriteScreen(lpath, fb2->fbmem, fb2->width, fb2->height, wakeSig);
                fb2->clear_screen();
            }
        }

        auto oPath = path;
        oPath += "_0";
        oPath.replace_extension(".png");
        ScreenCatcher::WriteScreen(oPath, fb->fbmem, fb->width, fb->height, wakeSig);
        while (!bufferPool.empty())
            bufferPool.pop();
        buffer_count = 0;
    }
}