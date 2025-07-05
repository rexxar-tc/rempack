//
// Created by brant on 7/5/25.
//

#pragma once
#include "widgets.h"
#include "display/overlay.h"
#include "opkg.h"
#include "display/list_box.h"

namespace widgets {
    /*
    * __________________________________________________
    * | These packages conflict with requested pkg {0}:|
    * | _____________________________________________  |
    * | | List                                      |  |
    * | | Box                                       |  |
    * | | Contents                                  |  |
    * | ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯  |
    * |  Do you want to remove these packages?         |
    * |                                                |
    * | [OK] [Abort]                                   |
    * ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
    */
    class ConflictDialog : public Overlay {
    public:
        ConflictDialog(int x, int y, int w, int h, const shared_ptr<package> &requested,
                       const std::vector<shared_ptr<package>> &conflicts);

        void build_dialog() override;
        void show() override;
        void on_reflow() override;

        void mark_redraw() override;

        void setCallback(const function<void(bool)> &callback);

    private:
        function<void(bool)> _callback;
        const int padding = 20;
        shared_ptr<ui::VerticalLayout> layout;
        ui::Text *t1 = nullptr;
        ListBox *l1 = nullptr;
        ui::Text *t2 = nullptr;
        bool _accepted = false;
        shared_ptr<package> request = nullptr;
        vector<shared_ptr<package>> conflicts;

        void on_button_selected(std::string s) override;

        void add_buttons_reflow(ui::HorizontalReflow *button_bar);

        vector<shared_ptr<package>> results;

        void run_uninstall();

        void update_texts();
    };
} // widgets
