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
}
