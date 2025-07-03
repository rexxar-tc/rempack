//
// Created by brant on 7/3/25.
//

#pragma once

#include "widgets.h"
#include "filter_options.h"
#include "display/list_box.h"

namespace widgets {
    class FilterOverlay: RoundCornerWidget{
    public:
        ui::Scene scene;
        shared_ptr<FilterOptions> options;
        FilterOverlay(int x, int y, int w, int h, shared_ptr<FilterOptions> currentOptions): RoundCornerWidget(x,y,w,h,RoundCornerStyle()) {
            options = currentOptions;
            scene = make_overlay();
        }

        void show() override;

        void render() override;

        void mark_redraw() override;

        PLS_DEFINE_SIGNAL(OFILTER_EVENT, FilterOptions);

        class OFILTER_EVENTS {
        public:
            OFILTER_EVENT updated;
        };

        OFILTER_EVENTS events;

    private:
        void upate_event();
        /*
         * [ ] Installed
         * [ ] Upgradable
         *
         * __________________
         * | Available      |
         * | Repository list|
         * ------------------
         *
         */
        shared_ptr<ListBox> _repoList;
        shared_ptr<ListBox> _licenseList;
        ui::Scene make_overlay();
    };

}