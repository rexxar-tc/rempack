//
// Created by brant on 7/3/25.
//

#include "install_dialog.h"
#include "../../../src/widget_helpers.h"
#include "terminal_dialog.h"
#include "display/list_box.h"

namespace widgets{

    void InstallDialog::build_dialog() {
        this->create_scene();
        int padding = 20;
        unordered_map<string, uint> items;
        for (const auto &pk: packages) {
            widget_helpers::format_deps_recursive(items, pk);
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
        int lh = min((int) 300, (int) ((labels.size()) * (utils::line_height() + 5)) + 10);
        auto l1 = new ListBox(dx, dy + padding, dw, lh, utils::line_height(), scene);
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

    void InstallDialog::setCallback(const function<void(bool)> &callback) {
        _callback = callback;
    }

    void InstallDialog::on_button_selected(std::string s) {
        if (s == "OK") {
            _accepted = true;
            run_install();
            return;
        } else {
            _accepted = false;
        }
        widgets::Overlay::on_button_selected(s);
        if (_callback)
            _callback(_accepted);
    }

    void InstallDialog::run_install() {
        auto td = new TerminalDialog(500, 500, 800, 1100, "opkg install");
        td->set_callback([this]() {
            if (this->_callback)
                this->_callback(this->_accepted);
            this->hide();
        });
        td->show();
        ui::TaskQueue::add_task([=]() {
            auto ret = opkg::Install(packages, [=](const string s) { td->stdout_callback(s); });
            if (ret == 0)
                td->stdout_callback("Done.");
            else
                td->stdout_callback("Error!");
            std::cout << "opkg install returned with exit code " << ret << std::endl;
        });
    }

}