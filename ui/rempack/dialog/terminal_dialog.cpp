//
// Created by brant on 7/3/25.
//

#include "terminal_dialog.h"

namespace widgets {
    void TerminalDialog::build_dialog() {
        this->create_scene();
        layout = make_shared<ui::VerticalLayout>(x, y, w, h, this->scene);
        int dx = padding;
        int dy = padding;
        int dw = w - padding - padding;
        t1 = new ui::Text(dx, dy, dw, utils::line_height(), title);
        int dht = h - padding - padding - 50 - utils::line_height();
        buffer_size = dht / (utils::line_height() + padding);
        layout->pack_start(t1);
        l1 = new ui::MultiText(dx, dy + padding, dw, dht, "Running...");
        layout->pack_start(l1);
        auto button_bar = new ui::HorizontalLayout(0, 0, this->w, 50, this->scene);
        layout->pack_end(button_bar);
        button_bar->y -= 2;

        // layout->reflow();

        this->add_buttons(button_bar);
        ui::TaskQueue::add_task([this]() { this->update_texts(); });
    }

    void TerminalDialog::on_reflow() {
        l1->on_reflow();
    }

    void TerminalDialog::mark_redraw() {
//layout->refresh();
        Overlay::mark_redraw();
    }

    void TerminalDialog::stdout_callback(const std::string &s) {
        //std::cout << s;
        ui::TaskQueue::add_task([=]() {
            auto lines = utils::wrap_string(s, w - padding - padding, ui::Style::DEFAULT.font_size);
            ui::TaskQueue::add_task([=]() {
                for (const auto &l: lines)
                    push_line(l);
                update_texts();
            });
        });
        //mark_redraw();
    }

    void TerminalDialog::set_callback(const std::function<void()> &cb) {
        callback = cb;
    }

    void TerminalDialog::on_button_selected(std::string s) {
        callback();
        widgets::Overlay::on_button_selected(s);
    }

    void TerminalDialog::add_buttons_reflow(ui::HorizontalReflow *button_bar) {
        auto default_fs = ui::Style::DEFAULT.font_size;
        for (auto b: this->buttons) {
            auto image = stbtext::get_text_size(b, default_fs);

            button_bar->pack_start(new ui::DialogButton(20, 0, image.w + default_fs, 50, this, b));
        }
    }

    void TerminalDialog::push_line(const std::string &l) {
        consoleBuffer.push_back(l);
        if (consoleBuffer.size() > buffer_size)
            consoleBuffer.pop_front();
    }


    void TerminalDialog::update_texts() {
        if (!t1 || !l1)
            return;


        std::stringstream ss;
        for (const auto &l: consoleBuffer)
            ss << l << std::endl;
        auto str = ss.str();
        l1->set_text(str.substr(0, str.size() - 2));
        l1->undraw();
        l1->mark_redraw();
        on_reflow();
        //Overlay::mark_redraw();
    }

    TerminalDialog::TerminalDialog(int x, int y, int w, int h, const string &title) : Overlay(x,y,w,h){
        this->buttons.clear();
        this->buttons.emplace_back("Dismiss");
        this->title = title;
    }
} // widgets