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

        void on_mouse_down(input::SynMotionEvent &ev) override{
            ev.stop_propagation();
            mark_redraw();
            fb->waveform_mode = WAVEFORM_MODE_DU;
        }

        void before_render() override{
            ui::Button::before_render();
            mouse_inside = mouse_down && mouse_inside;
        }

        void render_border() override{
            fb->draw_rect(x, y, w, h, GRAY, false);
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
    };

    class Keyboard : public ui::Widget {
        class KEYBOARD_EVENTS {
        public:
            KEYBOARD_EVENT changed;
            KEYBOARD_EVENT done;
        };

    private:

        enum KeyLayer {
            NONE, AlphaLow, AlphaUpper, Numeric, Symbols,
        };
        std::map<KeyLayer, ui::Scene> keyLayers;
        KeyLayer currentLayer = NONE;

        void lazy_init();
        void set_text(string t);
        void lower_layout();
        void upper_layout();
        void number_layout();
        void symbol_layout();
        void set_layout(KeyLayer layer);
        ui::Scene create_layout(string row1chars, string row2chars, string row3chars);
        KeyButton *make_char_button(char c);
        KeyButton *make_icon_button(icons::Icon icon, int w) const;

    public:
        bool shifted = false;
        bool numbers = false;
        vector<Row *> rows;
        ui::Scene scene;
        string text = "";
        int btn_width;
        int btn_height;

        ui::Stylesheet BTN_STYLE = ui::Stylesheet().font_size(48).valign_middle().justify_center();
        ui::Stylesheet INPUT_STYLE = ui::Stylesheet().font_size(64).underline();
        bool pinOverlay = false;

        KEYBOARD_EVENTS events;

        Keyboard(int x = 0, int y = 0, int w = 0, int h = 0) : Widget(x, y, w, h) {
            auto [dw, full_h] = fb->get_display_size();
            h = full_h / 4;
            this->w = dw;
            this->h = h;
            //lower_layout();
            lazy_init();
        }


        void render() override;
        void undraw() override;
        void show() override;
    };
}
