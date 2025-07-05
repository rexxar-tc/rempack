//
// Created by brant on 7/3/25.
//

#include "package_info_panel.h"

namespace widgets {
    const float rm_aspect = 0.75;
    void PackageInfoPanel::on_reflow() {
        _text->set_coords(x + padding, y + padding, w - (2 * padding), h - (2 * padding) - controlHeight);
        _text->mark_redraw();
        layout_buttons();
    }

    void PackageInfoPanel::set_text(string text) {
        _text->undraw();
        _text->text = std::move(text);
        _text->mark_redraw();
        this->mark_redraw();
    }

    void PackageInfoPanel::layout_image() {

    }

    void PackageInfoPanel::set_image(const shared_ptr<package>& package) {
        std::cout<<"image\n";
        auto data = opkg::getCachedSplashscreen(package);
        auto ic = ui::CachedIcon(data.data(), data.size(), package->Package.c_str(), _image->w, _image->h);
std::cout<<data.size()<<std::endl;
        _image->icon = ic;
        _image->on_reflow();
        _image->mark_redraw();
        _previewBtn->disable();
    }

    void PackageInfoPanel::display_package(const shared_ptr<package> &package) {
        if(package == nullptr){
            set_states(false);
            set_text("");
            undraw();
            mark_redraw();
            return;
        }
        _image->icon = ui::CachedIcon(nullptr, 0);
        bool splash = package->Section.rfind("splashscreens") != package->Section.npos;
        set_states(package->IsInstalled(), splash);
        set_text(opkg::FormatPackage(package));
        if(splash && opkg::isPackageCached(package))
            set_image(package);
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
} // widgets
