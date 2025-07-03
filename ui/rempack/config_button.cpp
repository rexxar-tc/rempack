//
// Created by brant on 7/3/25.
//

#include "config_button.h"
#include "rempack/overlay/menu_overlay.h"

namespace widgets{
    void ConfigButton::on_overlay_hidden(ui::InnerScene::DialogVisible v) {
        //events.updated(options);
    }

    void ConfigButton::on_mouse_click(input::SynMotionEvent &ev) {
        ev.stop_propagation();

        auto ov = new MenuOverlay(x - 500, y + h, 500, 800, data);
        ov->scene->on_hide += PLS_DELEGATE(on_overlay_hidden);
//ov->events.updated += [this, &ov](FilterOptions* o){events.updated(options);};
        ov->show();
        ui::MainLoop::refresh();
    }
}