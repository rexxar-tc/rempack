//
// Created by brant on 7/3/25.
//

#include "filter_button.h"
#include "overlay/filter_overlay.h"

namespace widgets{
    void FilterButton::on_overlay_hidden(ui::InnerScene::DialogVisible v) {
        //events.updated(options);
    }

    void FilterButton::on_mouse_click(input::SynMotionEvent &ev) {
        ev.stop_propagation();

        auto ov = new FilterOverlay(x + w, y + h, 500, 800, options);
        ov->scene->on_hide += [=](auto &d) { on_overlay_hidden(d); };
        ov->events.updated += [=](FilterOptions &o) { events.updated(o); };
        ov->show();
        ui::MainLoop::refresh();
    }

}