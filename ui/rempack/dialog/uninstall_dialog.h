//
// Created by brant on 7/3/25.
//

#pragma once

#include "display/overlay.h"
#include "opkg.h"
#include "display/list_box.h"

namespace widgets {
    /*
    * _________________________________________________
    * | Uninstalling %d packages and %d dependencies: |
    * | _____________________________________________ |
    * | | List                                      | |
    * | | Box                                       | |
    * | | Contents                                  | |
    * | ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯ |
    * |  [X] Autoremove dependencies                  |
    * |                                               |
    * | This will free approximately %d K/MB          |
    * | [OK] [Abort]                                  |
    * ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
    */
    class UninstallDialog: public widgets::Overlay {
    public:
        UninstallDialog(int x, int y, int w, int h, const std::vector<shared_ptr<package>> &toInstall): Overlay(x,y,w,h){
            packages = toInstall;
            pinned = true;
        }
        void build_dialog() override;
        void on_reflow() override;
        void mark_redraw() override;
        void setCallback(const function<void(bool)>& callback);

    private:
        function<void(bool)> _callback;
        const int padding = 20;
        shared_ptr<ui::VerticalLayout> layout;
        ui::Text *t1 = nullptr;
        ListBox *l1 = nullptr;
        ui::Text *t2 = nullptr;
        ui::ToggleButton *cb = nullptr;
        bool _accepted = false;
        std::vector<shared_ptr<package>> packages;
        bool dependencies = false;

        void on_button_selected(std::string s) override;
        void on_autoremove_tick(bool state);
        void add_buttons_reflow(ui::HorizontalReflow *button_bar);

        vector<shared_ptr<package>> results;

        void run_uninstall();
        void update_texts();
    };
} // widgets
