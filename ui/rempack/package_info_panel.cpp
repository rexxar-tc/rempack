//
// Created by brant on 7/3/25.
//

#include "package_info_panel.h"
#include "dispatcher.h"

namespace widgets {
    const float rm_aspect = 0.75;
    const icons::Icon syncIcon = ICON(assets::png_cloud_download_png);
    map<string, ui::CachedIcon> images {};

    void PackageInfoPanel::on_reflow() {
        layout_image();
        layout_buttons();
    }

    void PackageInfoPanel::set_text(const string& text) {
        _text->undraw();
        _text->text = text;
        _text->mark_redraw();
        this->mark_redraw();
    }

    void PackageInfoPanel::layout_image() {
        if(_image->visible) {
            int dw = (_image->x - _text->x) - (padding * 4);
            _text->set_coords(x+padding,y+padding, dw, _text->h);
        }
        else{
            _text->set_coords(x+padding,y+padding,w-(2*padding),h-(2*padding) - controlHeight);
        }
        _text->undraw();
        _text->mark_redraw();
        mark_redraw();
    }

    void PackageInfoPanel::set_image(const shared_ptr<package>& package) {
        _previewBtn->disable();
        _image->show();
        auto it = images.find(package->Package);
        if (it == images.end()) {
            _image->setImage(syncIcon, 100, 100);
            layout_image();
            ui::TaskQueue::add_task([=]() {
                vector<uint8_t> data;
                data = opkg::getCachedSplashscreen(package);
                int ix, iy, comp;
                bool decoded = stbi_info_from_memory(data.data(), data.size(), &ix, &iy, &comp);
                auto ic = images.emplace(package->Package,
                                         ui::CachedIcon(data.data(), data.size(), package->Package.c_str(), _image->w,
                                                        _image->h));
                widgets::Dispatcher::add_task([=]() {
                    if(decoded)
                        _image->setAspectWidth(ix, iy);
                    _image->setImage(ic.first->second);
                });
            });
        } else {
            auto ico = it->second;
            _image->setAspectWidth(ico.width, ico.height);
            _image->setImage(ico);
            layout_image();
        }
    }

    void PackageInfoPanel::display_package(const shared_ptr<package> &package) {
        if(package == nullptr){
            set_states(false);
            _image->hide();
            _text->hide();
            undraw();
            mark_redraw();
            return;
        }
        _text->show();
        _text->mark_redraw();
        bool splash = package->Section.rfind("splashscreens") != std::string::npos;
        set_states(package->IsInstalled(), splash);
        set_text(opkg::FormatPackage(package));
        if(splash && opkg::isPackageCached(package)) {
            _previewBtn->disable();
            set_image(package);
        }
        undraw();
        mark_redraw();
        on_reflow();
    }

    void PackageInfoPanel::set_states(bool installed, bool canPreview) {
        if (installed) {
            _installBtn->disable();
            _removeBtn->enable();
        } else {
            _installBtn->enable();
            _removeBtn->disable();
        }
        if (canPreview) {
            _previewBtn->enable();
            _previewBtn->show();
        }
        else {
            _previewBtn->disable();
            _previewBtn->hide();
            _image->hide();
        }
    }

    void PackageInfoPanel::layout_buttons() {
        auto dx = x + padding;
        auto dy = y + h - padding - controlHeight;
        auto dh = h - (padding * 6) - controlHeight;
        auto dw = (int)(floor(dh * rm_aspect));
        _installBtn->set_coords(dx, dy, controlWidth, controlHeight);
        dx += controlWidth + padding;
        _removeBtn->set_coords(dx, dy, controlWidth, controlHeight);
        dx += controlWidth + padding;
        _previewBtn->set_coords(dx, dy, controlWidth, controlHeight);

        _image->set_coords(w - dw + padding + padding, y + (padding * 2), dw, dh);

        _installBtn->on_reflow();
        _removeBtn->on_reflow();
        _previewBtn->on_reflow();
        _image->on_reflow();
        _image->mark_redraw();
        _installBtn->mark_redraw();
        _removeBtn->mark_redraw();
        _previewBtn->mark_redraw();
    }

    PackageInfoPanel::PackageInfoPanel(int x, int y, int w, int h, RoundCornerStyle style,
                                       shared_ptr<ui::InnerScene> &scene) : RoundCornerWidget(x,y,w,h,style){
        _text = make_shared<ui::MultiText>(x,y,w,h,"");
        _text->set_coords(x+padding,y+padding,w-(2*padding),h-(2*padding) - controlHeight);
        children.push_back(_text);
        _installBtn = make_shared<EventButton>(x,y,200, controlHeight,"Install");
        _removeBtn = make_shared<EventButton>(x,y,200, controlHeight,"Uninstall");
        _previewBtn = make_shared<EventButton>(x,y,200, controlHeight,"Preview");
        _image = make_shared<BorderedPixmap>(x,y,200,controlHeight, icons::Icon(), RoundCornerStyle());
        children.push_back(_installBtn);
        scene->add(_installBtn);
        children.push_back(_removeBtn);
        scene->add(_removeBtn);
        children.push_back(_previewBtn);
        scene->add(_previewBtn);
        children.push_back(_image);
        scene->add(_image);
        _installBtn->events.clicked += [this](void*){events.install();};
        _removeBtn->events.clicked += [this](void*){events.uninstall();};
        _previewBtn->events.clicked += [this](void*){events.preview();};
        layout_buttons();
    }

    void PackageInfoPanel::debugRender() {
        fb->draw_rect(_text->x, _text->y, _text->w, _text->h, toRColor(0,255,255), false);
        RoundCornerWidget::debugRender();
    }
} // widgets
