//
// Created by brant on 7/3/25.
//

#include "search_box.h"

namespace widgets {
    void SearchBox::on_reflow() {
        pixmap->set_coords(x + w - h, y, h, h);
        pixmap->mark_redraw();
        RoundedTextInput::on_reflow();
    }

    void SearchBox::on_mouse_click(input::SynMotionEvent &ev) {
        ev.stop_propagation();
        _keyboard->show();
    }

    void SearchBox::onChange(KeyboardEvent ev) {
        //need to do a full undraw in case characters were removed
        RoundedTextInput::undraw();
        set_text(_keyboard->text + '_');
        //redraw the button we just erased
        pixmap->mark_redraw();
    }

    void SearchBox::onDone(KeyboardEvent ev) {
        set_text(ev.text);
        on_done(ev.text);
    }

    SearchBox::SearchBox(int x, int y, int w, int h, RoundCornerStyle style, const string text) : RoundedTextInput(x, y, w, h, style, text) {
        //TODO: style sheets
        pixmap = make_shared<ui::Pixmap>(x + w - h, y, h, h, ICON(assets::png_search_png));
        children.push_back(pixmap);
        _keyboard = new Keyboard();
        _keyboard->pinOverlay = true;
        _keyboard->events.changed += PLS_DELEGATE(onChange);
        _keyboard->events.done += PLS_DELEGATE(onDone);

    }

} // widgets