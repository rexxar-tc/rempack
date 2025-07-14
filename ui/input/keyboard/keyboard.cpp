//
// Created by brant on 7/2/25.
//
#include "keyboard.h"
namespace widgets {
        void Keyboard::lazy_init() {
            ui::TaskQueue::add_task([this]() {
                keyLayers[AlphaLow] = create_layout("qwertyuiop", "asdfghjkl", "zxcvbnm");
                keyLayers[AlphaUpper] = create_layout("QWERTYUOIP", "ASDFGHJKL", "ZXCVBNM");
                keyLayers[Numeric] = create_layout("1234567890", "-/:;() &@\"", "  ,.?!'  ");
                keyLayers[Symbols] = create_layout("[]{}#%^*+=", "_\\|~<> $  ", "  ,.?!'  ");
            });
        }

        void Keyboard::set_text(string t) {
            text = t;
            //events.changed(t);
        };


        void Keyboard::lower_layout() {
            numbers = false;
            shifted = false;
            set_layout(AlphaLow);
        };

        void Keyboard::upper_layout() {
            numbers = false;
            shifted = true;
            set_layout(AlphaUpper);
        };

        void Keyboard::number_layout() {
            numbers = true;
            shifted = false;
            set_layout(Numeric);
        };

        void Keyboard::symbol_layout() {
            numbers = true;
            shifted = true;
            set_layout(Symbols);
        };

        void Keyboard::set_layout(KeyLayer layer) {
            if (currentLayer != NONE)
                ui::MainLoop::hide_overlay(keyLayers[currentLayer]);
            if (layer != NONE) {
                auto s = keyLayers[layer];
                s->pinned = pinOverlay;
                ui::MainLoop::show_overlay(s);
            }
            currentLayer = layer;
            //ui::MainLoop::refresh();
        }

        ui::Scene Keyboard::create_layout(const string& row1chars, const string& row2chars, const string& row3chars) {
            auto s = ui::make_scene();
            s->add(this);

            btn_width = w / row1chars.size();
            btn_height = 100;
            auto indent = row1chars.size() > row2chars.size() ? h / 8 : 0;
            auto row1 = new Row(0, 0, w, 100, s);
            auto row2 = new Row(indent, 0, w, 100, s);
            auto row3 = new Row(indent, 0, w, 100, s);
            auto row4 = new Row(0, 0, w, 100, s);

            auto structuredArgs_40 = fb->get_display_size();
            auto fw = get<0>(structuredArgs_40);
            auto fh = get<1>(structuredArgs_40);
            auto v_layout = ui::VerticalLayout(0, 0, fw, fh, s);

            v_layout.pack_end(row4);
            v_layout.pack_end(row3);
            v_layout.pack_end(row2);
            v_layout.pack_end(row1);

            for (auto c: row1chars) {
                row1->add_key(make_char_button(c));
            }

            for (auto c: row2chars) {
                row2->add_key(make_char_button(c));
            }

            auto shift_key = new KeyButton(0, 0, btn_width, btn_height, "shift");
            shift_key->set_style(BTN_STYLE);
            shift_key->mouse.click += PLS_LAMBDA(auto & ev)
            {
                if (!numbers and !shifted) {
                    upper_layout();
                } else if (!numbers) {
                    lower_layout();
                } else if (!shifted) {
                    symbol_layout();
                } else {
                    number_layout();
                }
            };
            row3->add_key(shift_key);
            for (auto c: row3chars) {
                row3->add_key(make_char_button(c));
            }
            auto backspace_key = new KeyButton(0, 0, btn_width, btn_height, "back");
            backspace_key->set_style(BTN_STYLE);


            backspace_key->mouse.click += PLS_LAMBDA(auto & ev)
            {
                if (text.size() > 0) {
                    text.pop_back();
                    dirty = 1;
                    events.changed();
                }
            };
            row3->add_key(backspace_key);

            auto kbd = new KeyButton(0, 0, btn_width, btn_height, "kbd");
            kbd->mouse.click += PLS_LAMBDA(auto & ev)
            {
                if (numbers) {
                    lower_layout();
                } else {
                    number_layout();
                }
            };
            auto space_key = new KeyButton(0, 0, btn_width * 8, btn_height, "space");
            space_key->set_style(BTN_STYLE);
            space_key->mouse.click += PLS_LAMBDA(auto & ev)
            {
                text += " ";
                dirty = 1;
                events.changed();
            };

            auto enter_key = new KeyButton(0, 0, btn_width, btn_height, "done");
            enter_key->set_style(BTN_STYLE);
            enter_key->mouse.click += PLS_LAMBDA(auto & ev)
            {
                hide();
                //ui::MainLoop::refresh();
                auto kev = KeyboardEvent{text};
                events.changed(kev);

                if (ui::MainLoop::hide_overlay(s) == nullptr) {
                    std::cerr << "No keyboard overlay to hide" << std::endl;
                }

                events.done(kev);
            };

            row4->add_key(kbd);
            row4->add_key(space_key);
            row4->add_key(enter_key);
            return s;
        };


        KeyButton *Keyboard::make_char_button(char c) {
            string s(1, c);
            auto key = new KeyButton(0, 0, btn_width, btn_height, s);
            key->set_style(BTN_STYLE);
            key->mouse.click += PLS_LAMBDA(auto & ev)
            {
                dirty = 1;
                if (c == ' ') {
                    return;
                }

                text.push_back(c);
                events.changed();
                //std::cerr << "key pressed:" << ' ' << c << std::endl;
            };
            return key;
        };

        KeyButton *Keyboard::make_icon_button(icons::Icon icon, int w) const {
            auto key = new KeyButton(0, 0, btn_width, btn_height, "");
            key->icon = icon;
            return key;
        };

        void Keyboard::render() {
            //fb->draw_rect(x, y, w, h, WHITE, true);
        };

        void Keyboard::show() {
            lower_layout();
        };

    void Keyboard::undraw() {
        Widget::undraw();
    }
}
