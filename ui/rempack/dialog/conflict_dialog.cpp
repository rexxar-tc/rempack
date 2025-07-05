//
// Created by brant on 7/5/25.
//

#include "conflict_dialog.h"
#include "terminal_dialog.h"

namespace widgets {
    ConflictDialog::ConflictDialog(int x, int y, int w, int h, const shared_ptr<package> &requested,
                                   const vector<shared_ptr<package>> &conflicts) : Overlay(x,y,w,h){
        this->request = requested;
        this->conflicts = conflicts;
    }

    void ConflictDialog::show() {
        if (!this->scene)
            build_dialog();
        visible = true;
        //mark_redraw();
        this->scene->pinned = pinned;
        ui::MainLoop::show_overlay(this->scene, stack);
    }

    void ConflictDialog::build_dialog() {
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
        t2 = new ui::Text(dx, dy + padding + padding, dw, utils::line_height(), "Do you want to remove these packages?");
        layout->pack_start(t2);
        auto button_bar = new ui::HorizontalLayout(0, 0, this->w, 50, this->scene);
        layout->pack_end(button_bar);
        button_bar->y -= 2;

// layout->reflow();

        this->add_buttons(button_bar);
        this->update_texts();
    }

    void ConflictDialog::on_reflow() {
        l1->on_reflow();
    }

    void ConflictDialog::mark_redraw() {
        layout->refresh();
    }

    void ConflictDialog::setCallback(const function<void(bool)> &callback) {
        _callback = callback;
    }

    void ConflictDialog::on_button_selected(std::string s) {
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

    void ConflictDialog::add_buttons_reflow(ui::HorizontalReflow *button_bar) {
        auto default_fs = ui::Style::DEFAULT.font_size;
        for (auto b: this->buttons) {
            auto image = stbtext::get_text_size(b, default_fs);

            button_bar->pack_start(new ui::DialogButton(20, 0, image.w + default_fs, 50, this, b));
        }
    }

    void ConflictDialog::run_uninstall() {
        auto td = new TerminalDialog(500, 500, 800, 1100, "opkg uninstall");
        td->set_callback([this]() {
            if (this->_callback)
                this->_callback(this->_accepted);
            this->hide();
        });
        td->show();
        auto ret = opkg::Uninstall(conflicts, [td](const string s) { td->stdout_callback(s); });
        if (ret == 0) {
            td->stdout_callback("Done.");
            for (const auto &t: conflicts)
                t->State = package::NotInstalled;
        } else {
            td->stdout_callback("Error!");
        }
        std::cout << "opkg uninstall returned with exit code " << ret << std::endl;
    }

    void ConflictDialog::update_texts() {
        if (!t1 || !t2 || !l1)
            return;
        results.clear();
        auto ret = opkg::Instance->ComputeUninstall(conflicts, false, &results);
        if (ret != 0) {
            printf("OPKG ERROR! %d\n", ret);
        }
        stringstream s1;
        s1 << results.size() << " packages conflict with request:";

        t1->set_text(s1.str());

        uint totalSize = 0;
        int lh = min((int) 300, (int) ((results.size()) * (utils::line_height() + 5)) + 10);
        l1->h = lh;
        l1->clear();
        for (const auto &pk: results) {
            l1->add(pk->Package);
            totalSize += pk->Size;
        }
        t2->y = lh + padding + l1->y;
        on_reflow();
        //Overlay::mark_redraw();
    }

    class Zyx{
    public:
        virtual void foo();
        virtual void bar();
    };

    class A: public Zyx{
    public:
        void foo() override{
            bar();
        }
        void bar() override{
            //dostuff
        }
    };

    class B: public A{
    public:
        void bar() override{
            //do stuff without calling A::bar
        }
    };
} // widgets