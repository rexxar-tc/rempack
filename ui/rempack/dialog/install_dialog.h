//
// Created by brant on 7/3/25.
//

#pragma once

#include "display/overlay.h"
#include "opkg.h"

namespace widgets{
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
            pinned = true;
        }
        void build_dialog() override;
        void setCallback(const function<void(bool)>& callback);
    private:
        function<void(bool)> _callback;
        std::vector<shared_ptr<package>> packages;
        bool _accepted = false;

        void on_button_selected(std::string s) override;
        void run_install();
    };
}
