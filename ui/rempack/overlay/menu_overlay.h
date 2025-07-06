//
// Created by brant on 7/3/25.
//

#pragma once
#include "widgets.h"
#include "menu_data.h"
namespace widgets {
    class MenuOverlay: RoundCornerWidget{
    public:
        ui::Scene scene;
        MenuData *data;
        MenuOverlay(int x, int y, int w, int h, MenuData *currentData);

        void show() override;
        void hide() override;
        void render() override;
        void mark_redraw() override;

        PLS_DEFINE_SIGNAL(OMENU_EVENT, MenuData*);

        class OMENU_EVENTS {
        public:
            OMENU_EVENT updated;
        };

        OMENU_EVENTS events;

    private:
        void upate_event();
        void refresh_event(void*);
        /*
         * [x] Check upgrades
         * UI Style:
         * (*) Cosy
         * ( ) Compact
         * Font Size [10[^v]]
         */
        ui::Scene make_overlay();
    protected:
        void debugRender() override;
    };

} // widgets
