//
// Created by brant on 7/3/25.
//

#include "package_info_panel.h"

namespace widgets {
    const float rm_aspect = 0.75;
    const icons::Icon syncIcon = ICON(assets::png_cloud_download_png);
    map<string, ui::CachedIcon> images {};
    int padding = 15;
    int controlHeight = 40;
    int controlWidth = 200;
    shared_ptr<ui::MultiText> _text;
    shared_ptr<EventButton> _installBtn, _removeBtn, _previewBtn;
    shared_ptr<BorderedPixmap> _image;
    shared_ptr<ui::VerticalReflow> _layout;

    void PackageInfoPanel::on_reflow() {
        layout_controls();
    }

    void PackageInfoPanel::set_text(const string& text) {
        _text->undraw();
        _text->text = text;
        _text->mark_redraw();
        this->mark_redraw();
    }

    void PackageInfoPanel::set_image(const shared_ptr<package>& package) {
        _previewBtn->disable();
        _image->show();
        auto it = images.find(package->Package);
        if (it == images.end()) {
            _image->setImage(syncIcon, 100, 100);
            layout_controls();
            ui::TaskQueue::add_task([=]() {
                vector<uint8_t> data;
                data = opkg::getCachedSplashscreen(package);
                int ix, iy, comp;
                bool decoded = stbi_info_from_memory(data.data(), data.size(), &ix, &iy, &comp);
                auto ic = images.emplace(package->Package,
                                         ui::CachedIcon(data.data(), data.size(), package->Package.c_str(), _image->getWidthForAspect(ix, iy), _image->h));
                ui::IdleQueue::add_task([=]() {
                    _image->undraw();
                    if(decoded)
                        _image->setAspectWidth(ix, iy);
                    _image->setImage(ic.first->second);
                    layout_controls();
                    _text->set_text(opkg::FormatPackage(package));
                });
            });
        } else {
            auto ico = it->second;
            _image->setAspectWidth(ico.width, ico.height);
            _image->setImage(ico);
            layout_controls();
            if(package != nullptr)
                _text->set_text(opkg::FormatPackage(package));
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
        else{
            _image->undraw();
            _image->hide();
        }
        //undraw();
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

    void PackageInfoPanel::layout_controls() {
        //undraw();
        auto lx = x+padding;
        auto ly = y+padding;
        auto dx = x + padding;
        auto dy = y + h - padding - controlHeight;
        _installBtn->set_coords(dx, dy, controlWidth, controlHeight);
        dx += controlWidth + padding;
        _removeBtn->set_coords(dx, dy, controlWidth, controlHeight);
        dx += controlWidth + padding;
        _previewBtn->set_coords(dx, dy, controlWidth, controlHeight);


        auto h1 = h-(3*padding) - controlHeight;
        if(_image->visible) {
            _image->undraw();
            _text->undraw();
            _image->set_coords(w - _image->w, ly, _image->w, h1);
            _text->set_coords(lx, ly, w - (padding * 4) - _image->w, h1);
            _image->on_reflow();
            _image->mark_redraw();
        }
        else{
            _text->set_coords(lx, ly, w - (padding * 2), h1);
        }

        mark_redraw();
        _text->mark_redraw();
        _text->on_reflow();
        _installBtn->on_reflow();
        _removeBtn->on_reflow();
        _previewBtn->on_reflow();
        _image->on_reflow();
        _image->mark_redraw();
        _installBtn->mark_redraw();
        _removeBtn->mark_redraw();
        _previewBtn->mark_redraw();
    }

    shared_ptr<ui::InnerScene> scene;

    PackageInfoPanel::PackageInfoPanel(int x, int y, int w, int h, RoundCornerStyle style) : ui::Widget(x,y,w,h), DebuggableWidget(x,y,w,h), RoundCornerWidget(x,y,w,h,style){
        auto lx = x+padding;
        auto ly = y+padding;
        auto lw = w-(2*padding);
        auto h1 = h-(4*padding) - controlHeight;
        _text = make_shared<ui::MultiText>(lx, ly, lw, h1, "");
        auto iq = (int)(h1 * 0.75f); //dummy aspect ratio of 3/4 like the RM2
        _image = make_shared<BorderedPixmap>(lx - iq, ly ,iq, h1, icons::Icon(), RoundCornerStyle());
        _image->hide();
        children.push_back(_text);

        ly = y + w - controlHeight - padding;
        controlWidth = min(controlWidth, lw / 4);
        scene = ui::make_scene();
        _installBtn = make_shared<EventButton>(lx,ly,controlWidth, controlHeight,"Install", LightButtonStyle());
        _removeBtn = make_shared<EventButton>(lx,ly,controlWidth, controlHeight,"Uninstall", LightButtonStyle());
        _previewBtn = make_shared<EventButton>(lx,ly,controlWidth, controlHeight,"Preview", LightButtonStyle());
        _installBtn->disable();
        _installBtn->border->show();
        children.push_back(_installBtn);
        scene->add(_installBtn);
        _removeBtn->disable();
        _removeBtn->border->show();
        children.push_back(_removeBtn);
        scene->add(_removeBtn);
        _previewBtn->hide();
        _previewBtn->border->hide();
        children.push_back(_previewBtn);
        scene->add(_previewBtn);
        children.push_back(_image);
        scene->add(_image);
        _installBtn->events.clicked += [this](void*){events.install();};
        _removeBtn->events.clicked += [this](void*){events.uninstall();};
        _previewBtn->events.clicked += [this](void*){events.preview();};
        layout_controls();
    }

    void PackageInfoPanel::debugRender() {
        if(_image->visible)
            _image->debugRender();
        fb->draw_rect(_text->x, _text->y, _text->w, _text->h, toRColor(0,255,255), false);
        RoundCornerWidget::debugRender();
    }

    void PackageInfoPanel::get_preview() {
    if(_previewBtn->visible && _previewBtn->is_enabled())
        events.preview();
    }
} // widgets
