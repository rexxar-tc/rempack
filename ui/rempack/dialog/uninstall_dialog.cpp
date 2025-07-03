//
// Created by brant on 7/3/25.
//

#include "uninstall_dialog.h"
#include "terminal_dialog.h"

namespace widgets {
    void UninstallDialog::build_dialog() {
        this->create_scene();
        layout = make_shared<ui::VerticalLayout>(x, y, w, h, this->scene);
        int dx = padding;
        int dy = padding;
        int dw = w - padding - padding;
        t1 = new ui::Text(dx, dy, dw, utils::line_height(), "Loading...");
        layout->pack_start(t1);
        l1 = new widgets::ListBox(dx, dy + padding, dw, utils::line_height(), utils::line_height(), scene);
        l1->selectable = false;
        layout->pack_start(l1);
        cb = new ui::ToggleButton(dx, dy + padding + padding, dw, utils::line_height(), "Auto-remove dependencies");
        cb->toggled = dependencies;
        cb->events.toggled += [this](bool s) { on_autoremove_tick(s); };
        cb->style.justify = ui::Style::JUSTIFY::LEFT;
        layout->pack_start(cb);
        t2 = new ui::Text(dx, dy + padding + padding, dw, utils::line_height(), "");
        layout->pack_start(t2);
        auto button_bar = new ui::HorizontalLayout(0, 0, this->w, 50, this->scene);
        layout->pack_end(button_bar);
        button_bar->y -= 2;

// layout->reflow();

        this->add_buttons(button_bar);
        ui::TaskQueue::add_task([this]() { this->update_texts(); });
    }

    void UninstallDialog::on_reflow() {
        l1->on_reflow();
    }

    void UninstallDialog::mark_redraw() {
        layout->refresh();
    }

    void UninstallDialog::setCallback(const function<void(bool)> &callback) {
        _callback = callback;
    }

    void UninstallDialog::on_button_selected(std::string s) {
        if (s == "OK") {
            _accepted = true;
            run_uninstall();
            return;
        } else
            _accepted = false;
        widgets::Overlay::on_button_selected(s);
        if (_callback)
            _callback(_accepted);
    }

    void UninstallDialog::on_autoremove_tick(bool state) {
        dependencies = state;
        ui::TaskQueue::add_task([this]() { this->update_texts(); });
    }

    void UninstallDialog::add_buttons_reflow(ui::HorizontalReflow *button_bar) {
        auto default_fs = ui::Style::DEFAULT.font_size;
        for (auto b: this->buttons) {
            auto image = stbtext::get_text_size(b, default_fs);

            button_bar->pack_start(new ui::DialogButton(20, 0, image.w + default_fs, 50, this, b));
        }
    }

    void UninstallDialog::run_uninstall() {
        auto td = new TerminalDialog(500, 500, 800, 1100, "opkg uninstall");
        td->set_callback([this]() {
            if (this->_callback)
                this->_callback(this->_accepted);
            this->hide();
        });
        td->show();
        auto ret = opkg::Uninstall(packages, [td](const string s) { td->stdout_callback(s); },
                                   dependencies ? " --autoremove" : "");
        if (ret == 0) {
            td->stdout_callback("Done.");
            for (const auto &t: packages)
                t->State = package::NotInstalled;
        } else {
            td->stdout_callback("Error!");
        }
        std::cout << "opkg uninstall returned with exit code " << ret << std::endl;
    }

    void UninstallDialog::update_texts() {
        if (!t1 || !t2 || !l1)
            return;
        results.clear();
        auto ret = opkg::Instance->ComputeUninstall(packages, dependencies, &results);
        if (ret != 0) {
            printf("OPKG ERROR! %d\n", ret);
        }
        stringstream s1;
        s1 << "Uninstalling " << packages.size() << " packages and ";
        s1 << results.size() - packages.size() << " dependencies:";

        t1->set_text(s1.str());

        uint totalSize = 0;
        int lh = min((int) 300, (int) ((results.size()) * (utils::line_height() + 5)) + 10);
        l1->h = lh;
        l1->clear();
        for (const auto &pk: results) {
            l1->add(pk->Package);
            totalSize += pk->Size;
        }
        cb->y = lh + padding + l1->y;
        cb->mark_redraw();
        stringstream s2;
        s2 << "This will free approximately " << utils::stringifySize(totalSize);
        t2->y = cb->y + padding + cb->h;
        t2->set_text(s2.str());
        on_reflow();
        //Overlay::mark_redraw();
    }
} // widgets