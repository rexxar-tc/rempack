//
// Created by brant on 2/17/24.
//

#pragma once

#include <rmkit.h>
#include "../opkg/opkg.h"
#include "../ui/rempack_widgets.h"
#include "overlay.h"

namespace widget_helpers{
    namespace internal{
        void format_deps_recursive(unordered_map<string, uint> &items, const shared_ptr<package> &pkg){
            if(pkg->IsInstalled())
                return;
            string size = pkg->Size == 0 ? "0" : utils::stringifySize(pkg->Size);
            stringstream ss;
            ss << pkg->Package << " - " << size;
            if(!items.emplace(ss.str(), pkg->Size).second)
                return;
            for(const auto &dpkg: pkg->Depends)
                format_deps_recursive(items, dpkg);
        }
    }

    /*
     * _______________________________________________
     * | Installing %d packages and %d dependencies: |
     * | ___________________________________________ |
     * | | List                                    | |
     * | | Box                                     | |
     * | | Contents                                | |
     * | ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯ |
     * | This will require approximately %d K/MB     |
     * | [OK] [Abort]                                |
     * ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
     */
class InstallDialog: public widgets::Overlay {
public:
    InstallDialog(int x, int y, int w, int h, const std::vector<shared_ptr<package>> &toInstall): Overlay(x,y,w,h){
        packages = toInstall;
    }
    void build_dialog() override {
        this->create_scene();
        int padding = 20;
        unordered_map<string, uint> items;
        for (const auto &pk: packages) {
            internal::format_deps_recursive(items, pk);
        }
        stringstream s1;
        s1 << "Installing " << packages.size() << " packages and ";
        s1 << items.size() - packages.size() << " dependencies:";
        uint totalSize = 0;
        vector<string> labels;
        for (const auto &[n, s]: items) {
            labels.push_back(n);
            totalSize += s;
        }
        stringstream s2;
        s2 << "This will require approximately " << utils::stringifySize(totalSize);

        auto layout = ui::VerticalLayout(x, y, w, h, this->scene);
        int dx = padding;
        int dy = padding;
        int dw = w - padding - padding;
        auto t1 = new ui::Text(dx, dy, dw, utils::line_height(), s1.str());
        layout.pack_start(t1);
        int lh = min((uint)300, ((labels.size() )* (utils::line_height()+5))+10);
        auto l1 = new widgets::ListBox(dx, dy+padding, dw, lh, utils::line_height());
        for (const auto &line: labels) {
            l1->add(line);
        }
        l1->selectable = false;
        layout.pack_start(l1);
        auto t2 = new ui::Text(dx, dy + padding + padding, dw, utils::line_height(), s2.str());
        layout.pack_start(t2);
        auto button_bar = ui::HorizontalLayout(0, 0, this->w, 50, this->scene);
        layout.pack_end(button_bar);
        button_bar.y -= 2;

        this->add_buttons(&button_bar);
    }
private:
    //ui::VerticalReflow* layout;
    std::vector<shared_ptr<package>> packages;

    //void add_buttons(ui::HorizontalReflow *button_bar) {
    //    auto default_fs = ui::Style::DEFAULT.font_size;
    //    for (auto b : this->buttons) {
    //        auto image = stbtext::get_text_size(b, default_fs);
//
    //        button_bar->pack_start(new ui::DialogButton(20, 0, image.w + default_fs, 50, this, b)); } }
};
}