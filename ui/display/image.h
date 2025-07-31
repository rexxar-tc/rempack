//
// Created by brant on 7/29/25.
//

#pragma once

#include "widgets.h"

namespace widgets {
    class image  {
    public:
        image(int x, int y, int w, int h);

        void render() ;
        void on_reflow() ;
        tuple<int, int> get_render_size();
        void resize(int w, int h);
    private:
       // ui::CachedIcon icon;
    };
}
