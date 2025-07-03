//
// Created by brant on 7/3/25.
//

#pragma once
#include "widgets.h"
#include "display/overlay.h"

namespace widgets {
    /*
    * _________________________________________________
    * | Window Title                                  |
    * | _____________________________________________ |
    * | | Scrolling                                 | |
    * | | Text                                      | |
    * | | Contents                                  | |
    * | ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯ |
    * | [Close]                                       |
    * ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
    */
    class TerminalDialog: public widgets::Overlay {
    public:
        TerminalDialog(int x, int y, int w, int h, const std::string& title);
        void build_dialog() override;
        void on_reflow() override;
        void mark_redraw() override;
        void stdout_callback(const std::string &s);
        void set_callback(const std::function<void()> &cb);
    private:
        const int padding = 20;
        shared_ptr<ui::VerticalLayout> layout;
        ui::Text *t1 = nullptr;
        ui::MultiText *l1 = nullptr;
        std::deque<std::string> consoleBuffer;
        int buffer_size = 16;
        std::function<void()> callback;

        void on_button_selected(std::string s) override;
        void add_buttons_reflow(ui::HorizontalReflow *button_bar);
        void push_line(const std::string &l);
        void update_texts();
    };
} // widgets
