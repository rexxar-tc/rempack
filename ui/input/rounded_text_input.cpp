//
// Created by brant on 7/3/25.
//

#include "rounded_text_input.h"

namespace widgets{
    void RoundedTextInput::on_reflow(){
        border->set_coords(x,y,w,h);
        border->mark_redraw();
    }

    void RoundedTextInput::render() {
        //bypass TextInput::render to hide the border
        ui::Text::render(); // NOLINT(*-parent-virtual-call)
    }

    RoundedTextInput::RoundedTextInput(int x, int y, int w, int h, RoundCornerStyle style, string text) : ui::TextInput(x,y,w,h,std::move(text)){
        //TODO: this is so bad
        //TODO: style sheets
        ui::TextInput::style.valign = ui::Style::MIDDLE;
        ui::TextInput::style.justify = ui::Style::LEFT;
        this->style = style;
        border = make_shared<RoundCornerWidget>(x,y,w,h,style);
        children.push_back(border);
    }
}
