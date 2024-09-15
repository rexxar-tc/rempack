//
// Created by brant on 2/13/24.
//

#pragma once

#include "widgets.h"

namespace widgets {
    class KeyboardEvent {
    public:
        string text;

        //explicit KeyboardEvent(string t) : text(t) {}
    };

    PLS_DEFINE_SIGNAL(KEYBOARD_EVENT, KeyboardEvent);

    class KeyButton : public ui::Button {
    public:
        KeyButton(int x, int y, int w, int h, string t) : ui::Button(x, y, w, h, t) {
            (void) 0;
        }

        void before_render() {
            ui::Button::before_render();
            mouse_inside = mouse_down && mouse_inside;
        }
    };


    class Row : public ui::Widget {
    public:
        ui::HorizontalLayout *layout = NULL;
        ui::Scene scene;

        Row(int x, int y, int w, int h, ui::Scene s) : Widget(x, y, w, h) {
            scene = s;
            //scene->clear_under = true;
        }

        void add_key(KeyButton *key) {
            if (layout == NULL) {
                //std::cerr << "RENDERING ROW" << ' ' << x << ' ' << y << ' ' << w << ' ' << h << std::endl;
                layout = new ui::HorizontalLayout(x, y, w, h, scene);
            }
            layout->pack_start(key);
        }

        void render() {
            (void) 0;
        }
    };

    class Keyboard : public ui::Widget {
        class KEYBOARD_EVENTS {
        public:
            KEYBOARD_EVENT changed;
            KEYBOARD_EVENT done;
        };
    private:

        enum KeyLayer{
            NONE,
            AlphaLow,
            AlphaUpper,
            Numeric,
            Symbols,
        };
        std::map<KeyLayer, ui::Scene> keyLayers;
        KeyLayer currentLayer = NONE;

    public:
        bool shifted = false;
        bool numbers = false;
        vector<Row *> rows;
        ui::Scene scene;
        string text = "";
        int btn_width;
        int btn_height;

        ui::Stylesheet BTN_STYLE= ui::Stylesheet().font_size(48).valign_middle().justify_center();
        ui::Stylesheet INPUT_STYLE= ui::Stylesheet().font_size(64).underline();
        bool pinOverlay = false;

        KEYBOARD_EVENTS events;

        Keyboard(int x = 0, int y = 0, int w = 0, int h = 0) : Widget(x, y, w, h) {
            auto [dw, full_h]= fb->get_display_size();
            h = full_h / 4;
            this->w = dw;
            this->h = h;
            //lower_layout();
            lazy_init();
        };

        void lazy_init(){
            ui::TaskQueue::add_task([this](){keyLayers[AlphaLow] = create_layout("qwertyuiop", "asdfghjkl", "zxcvbnm");});
            ui::TaskQueue::add_task([this](){keyLayers[AlphaUpper] = create_layout("QWERTYUOIP", "ASDFGHJKL", "ZXCVBNM");});
            ui::TaskQueue::add_task([this](){keyLayers[Numeric] = create_layout("1234567890", "-/:;() &@\"", "  ,.?!'  ");});
            ui::TaskQueue::add_task([this](){keyLayers[Symbols] = create_layout("[]{}#%^*+=", "_\\|~<> $  ", "  ,.?!'  ");});
        }

        void set_text(string t) {
            text = t;
        };


        void lower_layout() {
            numbers = false;
            shifted = false;
            set_layout(AlphaLow);
        };

        void upper_layout() {
            numbers = false;
            shifted = true;
            set_layout(AlphaUpper);
        };

        void number_layout() {
            numbers = true;
            shifted = false;
            set_layout(Numeric);
        };

        void symbol_layout() {
            numbers = true;
            shifted = true;
            set_layout(Symbols);
        };

        void set_layout(KeyLayer layer){
            if(currentLayer != NONE)
                ui::MainLoop::hide_overlay(keyLayers[currentLayer]);
            if(layer != NONE) {
                auto s = keyLayers[layer];
                s->pinned = pinOverlay;
                ui::MainLoop::show_overlay(s);
            }
            currentLayer = layer;
            ui::MainLoop::refresh();
        }


        ui::Scene create_layout(string row1chars, string row2chars, string row3chars) {
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
            shift_key->mouse.click += PLS_LAMBDA(auto &ev) {
                if (!numbers and !shifted) {
                    upper_layout();
                } else if (!numbers and shifted) {
                    lower_layout();
                } else if (numbers and !shifted) {
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


            backspace_key->mouse.click += PLS_LAMBDA(auto &ev) {
                if (text.size() > 0) {
                    text.pop_back();
                    dirty = 1;
                }
            };
            row3->add_key(backspace_key);

            auto kbd = new KeyButton(0, 0, btn_width, btn_height, "kbd");
            kbd->mouse.click += PLS_LAMBDA(auto &ev) {
                if (numbers) {
                    lower_layout();
                } else {
                    number_layout();
                }
            };
            auto space_key = new KeyButton(0, 0, btn_width * 8, btn_height, "space");
            space_key->set_style(BTN_STYLE);
            space_key->mouse.click += PLS_LAMBDA(auto &ev) {
                text += " ";
                dirty = 1;
            };

            auto enter_key = new KeyButton(0, 0, btn_width, btn_height, "done");
            enter_key->set_style(BTN_STYLE);
            enter_key->mouse.click += PLS_LAMBDA(auto &ev) {
                hide();
                ui::MainLoop::refresh();
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


        KeyButton *make_char_button(char c) {
            string s(1, c);
            auto key = new KeyButton(0, 0, btn_width, btn_height, s);
            key->set_style(BTN_STYLE);
            key->mouse.click += PLS_LAMBDA(auto &ev) {
                dirty = 1;
                if (c == ' ') {
                    return;
                }

                text.push_back(c);
                std::cerr << "key pressed:" << ' ' << c << std::endl;
            };
            return key;
        };

        KeyButton *make_icon_button(icons::Icon icon, int w) {
            auto key = new KeyButton(0, 0, btn_width, btn_height, "");
            key->icon = icon;
            return key;
        };

        void render() {
            fb->draw_rect(x, y, w, h, WHITE, true);
        };

        void show() {
            lower_layout();
        };
    };
}